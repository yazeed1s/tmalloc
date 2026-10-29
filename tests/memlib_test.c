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
#include "memlib.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int
main(void)
{
	size_t page = (size_t)sysconf(_SC_PAGESIZE);

	assert(tmm_mem_init() == 0);
	assert(tmm_mem_size() == 0);

	/* first grow maps one page */
	char *a = (char *)tmm_sbrk(100);
	assert(a == tmm_mem_lo());
	assert(tmm_mem_size() == 100);
	assert(tmm_mem_syscalls() == 1);
	memset(a, 0xab, 100);

	/* still inside the same page, no syscall */
	char *b = (char *)tmm_sbrk(200);
	assert(b == a + 100);
	assert(tmm_mem_syscalls() == 1);

	/* crossing into new pages: one more syscall, all of it writable */
	char *c = (char *)tmm_sbrk((intptr_t)(3 * page));
	assert(c == b + 200);
	assert(tmm_mem_syscalls() == 2);
	memset(c, 0xcd, 3 * page);
	assert((char *)tmm_mem_hi() == c + 3 * page - 1);
	assert(tmm_mem_sbrk_calls() == 3);

	/* bad requests */
	assert(tmm_sbrk(-1) == (void *)-1);
	assert(tmm_sbrk((intptr_t)MEM_RESERVE) == (void *)-1);

	/* reset keeps the pages warm: same base, counters cleared, and growing
	 * back over memory that is already read/write needs no syscall */
	assert(tmm_mem_init() == 0);
	assert(tmm_mem_size() == 0);
	assert(tmm_mem_syscalls() == 0);
	char *d = (char *)tmm_sbrk((intptr_t)(2 * page));
	assert(d == a);
	assert(tmm_mem_syscalls() == 0);
	assert((unsigned char)d[0] == 0xab); /* still resident, not refaulted */

	/* past the old read/write end it mprotects again */
	assert(tmm_sbrk((intptr_t)(4 * page)) != (void *)-1);
	assert(tmm_mem_syscalls() == 1);

	printf("memlib_test: ok\n");
	return 0;
}
