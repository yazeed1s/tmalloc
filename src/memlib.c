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

#include "memlib.h"
#include <errno.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#define MAP_FLAGS (MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE)

static char	   *heap_base = NULL; /* start of the reservation */
static char	   *heap_brk  = NULL; /* first byte past the heap */
static char	   *heap_rw	  = NULL; /* first byte past the read/write pages */
static size_t	page_size = 0;
static uint64_t sbrk_calls;
static uint64_t syscalls;

int
tmm_mem_init(void)
{
	if (heap_base == NULL) {
		page_size = (size_t)sysconf(_SC_PAGESIZE);
		void *p	  = mmap(NULL, MEM_RESERVE, PROT_NONE, MAP_FLAGS, -1, 0);
		if (p == MAP_FAILED) {
			perror("memlib: mmap reserve");
			return -1;
		}
		heap_base = p;
		heap_rw	  = heap_base;
	}

	/*
	 * a reset keeps the pages that are already read/write and resident, only
	 * the break goes back to the start. throwing them away made every timed
	 * run fault the whole heap in again while glibc's heap stayed warm, which
	 * is a benchmark artifact, not something a real program ever pays.
	 */
	heap_brk   = heap_base;
	sbrk_calls = 0;
	syscalls   = 0;
	return 0;
}

/* grows the heap by incr bytes and returns the old brk, (void *)-1 on error.
 * shrinking is not supported. */
void *
tmm_sbrk(intptr_t incr)
{
	if (incr < 0 ||
		(size_t)incr > MEM_RESERVE - (size_t)(heap_brk - heap_base)) {
		errno = ENOMEM;
		return (void *)-1;
	}

	char *old	  = heap_brk;
	char *new_brk = heap_brk + incr;
	sbrk_calls++;

	if (new_brk > heap_rw) {
		uintptr_t end = ((uintptr_t)new_brk + page_size - 1) & ~(page_size - 1);
		size_t	  len = end - (uintptr_t)heap_rw;
		if (mprotect(heap_rw, len, PROT_READ | PROT_WRITE) != 0) {
			perror("memlib: mprotect");
			return (void *)-1;
		}
		syscalls++;
		heap_rw = (char *)end;
	}

	heap_brk = new_brk;
	return old;
}

void *
tmm_mem_lo(void)
{
	return heap_base;
}

/* last byte of the heap */
void *
tmm_mem_hi(void)
{
	return heap_brk - 1;
}

size_t
tmm_mem_size(void)
{
	return (size_t)(heap_brk - heap_base);
}

uint64_t
tmm_mem_sbrk_calls(void)
{
	return sbrk_calls;
}

uint64_t
tmm_mem_syscalls(void)
{
	return syscalls;
}
