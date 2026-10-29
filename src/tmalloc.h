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

#ifndef TMALLOC_TMALLOC_H
#define TMALLOC_TMALLOC_H

#include <stddef.h>
#include <stdint.h>

#define TMM_ALIGNMENT 16 /* every payload is aligned to this */

/* counters every version keeps, they explain the benchmark numbers */
typedef struct {
	uint64_t sbrk_calls; /* times the allocator asked memlib for more heap */
	uint64_t syscalls;	 /* mprotect calls memlib made to back that heap */
	uint64_t blocks_scanned; /* blocks visited while searching for a fit */
	uint64_t splits;		 /* blocks split on placement */
	uint64_t coalesces;		 /* merges with a free neighbor */
} tmm_stats_t;

/*
 * every version (implicit.c, explicit.c, segregated.c, glibc.c) implements
 * this same api, the bench driver is linked against one of them at a time.
 * tmm_init() resets the heap, so it can be called before every run.
 */
/* clang-format off */
int tmm_init(void);
void *tmm_malloc(size_t size);
void tmm_free(void *ptr);
int tmm_check(void);
size_t tmm_heap_size(void);
tmm_stats_t tmm_get_stats(void);
const char *tmm_name(void);
/* clang-format on */

#endif /* TMALLOC_TMALLOC_H */
