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

#include "tmalloc.h"
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>

/* baseline: the same api, straight onto glibc. there is no way to reset
 * glibc's heap, so tmm_init() does nothing and state carries between runs. */

int
tmm_init(void)
{
	return 0;
}

void *
tmm_malloc(size_t size)
{
	return malloc(size);
}

void
tmm_free(void *ptr)
{
	free(ptr);
}

int
tmm_check(void)
{
	return 0;
}

/*
 * sbrk heap + chunks glibc mmapped on its own, the same as mallinfo2()'s
 * arena + hblkhd. but mallinfo2() walks every bin, and after every op of a
 * 128k-live trace that took minutes. single-threaded, the main arena is one
 * contiguous brk heap, so its size is the break minus where it started, which
 * costs nothing. only the mmapped part needs mallinfo2(): every call while
 * any is around (few chunks then, so it is cheap), every 1024th otherwise.
 */
static char	   *heap_start = NULL;
static size_t	mmapped;
static unsigned calls;

size_t
tmm_heap_size(void)
{
	if (heap_start == NULL) {
		struct mallinfo2 mi = mallinfo2();
		heap_start			= (char *)sbrk(0) - mi.arena;
		mmapped				= mi.hblkhd;
	} else if (mmapped > 0 || ++calls % 1024 == 0) {
		mmapped = mallinfo2().hblkhd;
	}
	return (size_t)((char *)sbrk(0) - heap_start) + mmapped;
}

tmm_stats_t
tmm_get_stats(void)
{
	return (tmm_stats_t){0};
}

const char *
tmm_name(void)
{
	return "glibc";
}
