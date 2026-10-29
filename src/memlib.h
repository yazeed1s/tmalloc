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

#ifndef TMALLOC_MEMLIB_H
#define TMALLOC_MEMLIB_H

#include <stddef.h>
#include <stdint.h>

#define MEM_RESERVE ((size_t)1 << 32) /* 4 GiB of address space, not memory */

/*
 * the heap lives in one PROT_NONE reservation, tmm_sbrk() moves brk up and
 * mprotects pages to read/write as they are needed. the first tmm_mem_init()
 * reserves the region, later calls move the break back to the start and keep
 * the pages already made read/write (they stay resident, like a warm heap).
 */
/* clang-format off */
int tmm_mem_init(void);
void *tmm_sbrk(intptr_t incr);
void *tmm_mem_lo(void);
void *tmm_mem_hi(void);
size_t tmm_mem_size(void);
uint64_t tmm_mem_sbrk_calls(void);
uint64_t tmm_mem_syscalls(void);
/* clang-format on */

#endif /* TMALLOC_MEMLIB_H */
