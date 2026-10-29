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

#undef NDEBUG /* the asserts have side effects, keep them in every build */
#include "tmalloc.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*
 * corrupts the heap on purpose and makes sure tmm_check() notices. relies only
 * on the boundary-tag layout every version shares: 8-byte header right before
 * the payload, 8-byte footer at the end of the block, size | alloc bit.
 */

#define HDR(p) ((uint64_t *)((char *)(p) - 8))
#define FTR(p) ((uint64_t *)((char *)(p) + (*HDR(p) & ~(uint64_t)0xf) - 16))

/* tmm_check() prints what it found, keep the test output readable */
static void
expect_fail(const char *what)
{
	fprintf(stderr, "  expected failure (%s): ", what);
	assert(tmm_check() != 0);
}

int
main(void)
{
	assert(tmm_init() == 0);
	char *a = (char *)tmm_malloc(100);
	char *b = (char *)tmm_malloc(100);
	char *c = (char *)tmm_malloc(100);
	assert(a && b && c);
	assert(tmm_check() == 0);

	uint64_t saved = *HDR(b);
	*HDR(b) |= 0x4;
	expect_fail("stray header bits");
	*HDR(b) = saved;
	assert(tmm_check() == 0);

	saved	= *FTR(b);
	*FTR(b) = saved + 16;
	expect_fail("footer differs from header");
	*FTR(b) = saved;
	assert(tmm_check() == 0);

	/* mark a and b free without coalescing them */
	*HDR(a) &= ~(uint64_t)1;
	*FTR(a) &= ~(uint64_t)1;
	*HDR(b) &= ~(uint64_t)1;
	*FTR(b) &= ~(uint64_t)1;
	expect_fail("adjacent free blocks");

	/* free-list versions keep next/prev in the first two payload words */
	if (strcmp(tmm_name(), "implicit") != 0) {
		assert(tmm_init() == 0);
		a		= (char *)tmm_malloc(100);
		b		= (char *)tmm_malloc(100);
		c		= (char *)tmm_malloc(100);
		char *d = (char *)tmm_malloc(100);
		assert(tmm_check() == 0);

		/* tags say free, but the block never went on a list */
		*HDR(b) &= ~(uint64_t)1;
		*FTR(b) &= ~(uint64_t)1;
		expect_fail("free block not on a list");
		*HDR(b) |= 1;
		*FTR(b) |= 1;
		assert(tmm_check() == 0);

		/* b and d are free and not adjacent, so both are list nodes */
		tmm_free(b);
		tmm_free(d);
		assert(tmm_check() == 0);
		void **links	  = (void **)b;
		void  *saved_prev = links[1];
		links[1]		  = b;
		expect_fail("broken prev link");
		links[1] = saved_prev;
		assert(tmm_check() == 0);
		(void)a;
		(void)c;
	}

	/* segregated: a block whose size no longer matches its class list */
	if (strncmp(tmm_name(), "segregated", 10) == 0) {
		assert(tmm_init() == 0);
		a		= (char *)tmm_malloc(100); /* 128 B blocks, class [128, 256) */
		b		= (char *)tmm_malloc(100);
		c		= (char *)tmm_malloc(100);
		char *d = (char *)tmm_malloc(100);
		tmm_free(b);
		assert(tmm_check() == 0);

		/* fold c into b in the tags only: the heap walk stays valid, but b is
		 * now 256 B and still sits on the [128, 256) list */
		*HDR(b) = 256;
		*FTR(b) = 256;
		expect_fail("block on the wrong class list");
		(void)a;
		(void)d;
	}

	/* a clean heap again, then normal frees must keep it consistent */
	assert(tmm_init() == 0);
	a = (char *)tmm_malloc(100);
	b = (char *)tmm_malloc(100);
	c = (char *)tmm_malloc(100);
	tmm_free(b);
	assert(tmm_check() == 0);
	tmm_free(a);
	assert(tmm_check() == 0);
	tmm_free(c);
	assert(tmm_check() == 0);

	printf("check_test (%s): ok\n", tmm_name());
	return 0;
}
