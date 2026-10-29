/*
 * BSD 2-Clause License
 * Copyright (c) 2026, Yazeed Alharthi
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *	  1. Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 *
 *	  2. Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * implicit free list: every block, allocated or free, carries a header and a
 * footer (boundary tags) holding its size and an allocated bit. finding a fit
 * walks every block in address order, which is what makes this version slow.
 *
 * block layout (sizes in bytes, everything 16-byte aligned):
 *
 *	 | header 8 | payload ... | footer 8 |
 *	            ^ bp
 *
 * heap layout (pad, prologue and epilogue are 8 bytes each):
 *
 *	 | pad | prologue hdr | prologue ftr | block | ... | block | epilogue hdr |
 *
 * the prologue and epilogue are permanently allocated, so coalesce() never has
 * to special-case the first or last block.
 */

#include "memlib.h"
#include "tmalloc.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define WSIZE			  8			/* header / footer size */
#define DSIZE			  16		/* alignment, also header + footer */
#define MIN_BLOCK		  32		/* header + footer + 16 bytes of payload */
#define CHUNKSIZE		  (1 << 16) /* grow the heap by at least this much */

#define MAX(a, b)		  ((a) > (b) ? (a) : (b))
#define ALIGN(n)		  (((n) + (DSIZE - 1)) & ~(size_t)(DSIZE - 1))

#define PACK(size, alloc) ((uint64_t)(size) | (uint64_t)(alloc))
#define GET(p)			  (*(uint64_t *)(p))
#define PUT(p, val)		  (*(uint64_t *)(p) = (uint64_t)(val))
#define GET_SIZE(p)		  ((size_t)(GET(p) & ~(uint64_t)0xf))
#define GET_ALLOC(p)	  ((bool)(GET(p) & 0x1))

/* bp is always a payload pointer */
#define HDRP(bp)		  ((char *)(bp) - WSIZE)
#define FTRP(bp)		  ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
#define NEXT_BLKP(bp)	  ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp)	  ((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))

static char		  *heap_listp = NULL; /* prologue payload */
static tmm_stats_t stats;

/* clang-format off */
static void *extend_heap(size_t size);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
static int check_fail(const void *bp, const char *msg);
/* clang-format on */

int
tmm_init(void)
{
	if (tmm_mem_init() != 0)
		return -1;
	stats	= (tmm_stats_t){0};

	char *p = (char *)tmm_sbrk(4 * WSIZE);
	if (p == (void *)-1)
		return -1;

	PUT(p, 0);							/* pad, so payloads are 16-aligned */
	PUT(p + 1 * WSIZE, PACK(DSIZE, 1)); /* prologue header */
	PUT(p + 2 * WSIZE, PACK(DSIZE, 1)); /* prologue footer */
	PUT(p + 3 * WSIZE, PACK(0, 1));		/* epilogue header */
	heap_listp = p + 2 * WSIZE;
	return 0;
}

/* size must be a multiple of DSIZE. the new block's header takes the place of
 * the old epilogue, and a new epilogue goes right after it. */
static void *
extend_heap(size_t size)
{
	char *bp = (char *)tmm_sbrk((intptr_t)size);
	if (bp == (void *)-1)
		return NULL;

	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
	PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
	return coalesce(bp);
}

static void *
coalesce(void *bp)
{
	bool   prev_alloc = GET_ALLOC((char *)bp - DSIZE); /* prev block's footer */
	bool   next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	size_t size		  = GET_SIZE(HDRP(bp));

	if (prev_alloc && next_alloc)
		return bp;

	if (prev_alloc && !next_alloc) {
		/* merge with next: the header stays, the footer moves */
		size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
		stats.coalesces++;
	} else if (!prev_alloc && next_alloc) {
		/* merge with prev: bp's footer stays, prev's header grows */
		char *prev = PREV_BLKP(bp);
		size += GET_SIZE(HDRP(prev));
		PUT(FTRP(bp), PACK(size, 0));
		PUT(HDRP(prev), PACK(size, 0));
		bp = prev;
		stats.coalesces++;
	} else {
		char *prev = PREV_BLKP(bp);
		char *next = NEXT_BLKP(bp);
		size += GET_SIZE(HDRP(prev)) + GET_SIZE(HDRP(next));
		PUT(HDRP(prev), PACK(size, 0));
		PUT(FTRP(next), PACK(size, 0));
		bp = prev;
		stats.coalesces += 2;
	}
	return bp;
}

/* first fit over every block, allocated ones included */
static void *
find_fit(size_t asize)
{
	for (char *bp = NEXT_BLKP(heap_listp); GET_SIZE(HDRP(bp)) > 0;
		 bp		  = NEXT_BLKP(bp)) {
		stats.blocks_scanned++;
		if (!GET_ALLOC(HDRP(bp)) && GET_SIZE(HDRP(bp)) >= asize)
			return bp;
	}
	return NULL;
}

/* split only if the leftover can hold a block of its own */
static void
place(void *bp, size_t asize)
{
	size_t csize = GET_SIZE(HDRP(bp));

	if (csize - asize >= MIN_BLOCK) {
		PUT(HDRP(bp), PACK(asize, 1));
		PUT(FTRP(bp), PACK(asize, 1));
		char *rest = NEXT_BLKP(bp);
		PUT(HDRP(rest), PACK(csize - asize, 0));
		PUT(FTRP(rest), PACK(csize - asize, 0));
		stats.splits++;
	} else {
		PUT(HDRP(bp), PACK(csize, 1));
		PUT(FTRP(bp), PACK(csize, 1));
	}
}

void *
tmm_malloc(size_t size)
{
	if (size == 0 || size > MEM_RESERVE)
		return NULL;
	if (heap_listp == NULL && tmm_init() != 0)
		return NULL;

	size_t asize = MAX(ALIGN(size + DSIZE), (size_t)MIN_BLOCK);
	char  *bp	 = (char *)find_fit(asize);
	if (bp == NULL) {
		bp = (char *)extend_heap(MAX(asize, (size_t)CHUNKSIZE));
		if (bp == NULL)
			return NULL;
	}
	place(bp, asize);
	return bp;
}

void
tmm_free(void *ptr)
{
	if (ptr == NULL)
		return;

	size_t size = GET_SIZE(HDRP(ptr));
	PUT(HDRP(ptr), PACK(size, 0));
	PUT(FTRP(ptr), PACK(size, 0));
	coalesce(ptr);
}

static int
check_fail(const void *bp, const char *msg)
{
	fprintf(stderr,
			"tmm_check: %s at %p (offset %td)\n",
			msg,
			bp,
			(const char *)bp - (const char *)tmm_mem_lo());
	return -1;
}

/* walks the whole heap, returns 0 if it is consistent */
int
tmm_check(void)
{
	if (heap_listp == NULL)
		return 0;

	char *lo  = (char *)tmm_mem_lo();
	char *brk = (char *)tmm_mem_hi() + 1;

	if (GET(HDRP(heap_listp)) != PACK(DSIZE, 1) ||
		GET(FTRP(heap_listp)) != PACK(DSIZE, 1))
		return check_fail(heap_listp, "bad prologue");

	bool  prev_free = false;
	char *bp		= NEXT_BLKP(heap_listp);
	while (GET_SIZE(HDRP(bp)) > 0) {
		size_t size = GET_SIZE(HDRP(bp));

		if ((uintptr_t)bp % DSIZE != 0)
			return check_fail(bp, "payload not 16-byte aligned");
		if (GET(HDRP(bp)) & 0xe)
			return check_fail(bp, "stray bits in header");
		if (size < MIN_BLOCK)
			return check_fail(bp, "block smaller than the minimum");
		if (HDRP(bp) < lo || FTRP(bp) + WSIZE > brk - WSIZE)
			return check_fail(bp, "block runs outside the heap");
		if (GET(HDRP(bp)) != GET(FTRP(bp)))
			return check_fail(bp, "header and footer differ");

		bool is_free = !GET_ALLOC(HDRP(bp));
		if (is_free && prev_free)
			return check_fail(bp, "two adjacent free blocks");
		prev_free = is_free;
		bp		  = NEXT_BLKP(bp);
	}

	if (HDRP(bp) != brk - WSIZE)
		return check_fail(bp, "epilogue is not at the end of the heap");
	if (!GET_ALLOC(HDRP(bp)))
		return check_fail(bp, "epilogue is not marked allocated");
	return 0;
}

size_t
tmm_heap_size(void)
{
	return tmm_mem_size();
}

tmm_stats_t
tmm_get_stats(void)
{
	tmm_stats_t s = stats;
	s.sbrk_calls  = tmm_mem_sbrk_calls();
	s.syscalls	  = tmm_mem_syscalls();
	return s;
}

const char *
tmm_name(void)
{
	return "implicit";
}
