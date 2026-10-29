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

#define _GNU_SOURCE
#include "tmalloc.h"
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/perf_event.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_RUNS 10 /* timed runs, the median is reported */
#define CSV_HEADER                                                             \
	"version,trace,ops,ops_per_sec,peak_live,peak_heap,peak_util,rss_kb,"      \
	"sbrk_calls,syscalls,blocks_scanned,splits,coalesces,reserved_kb,"         \
	"cycles_per_op,instr_per_op,branch_miss_per_op,l1d_miss_per_op,"           \
	"cache_miss_per_op,dtlb_miss_per_op,page_faults_per_op"
#define HW_EVENTS 7 /* see hw_open() for the list */

typedef enum {
	OP_ALLOC,
	OP_FREE
} op_kind_t;

typedef struct {
	op_kind_t kind;
	uint32_t  id;
	uint32_t  size; /* only used by OP_ALLOC */
} op_t;

typedef struct {
	op_t	   *ops;
	size_t		n_ops;
	size_t		cap;
	uint32_t	n_ids; /* largest id + 1 */
	const char *path;
} trace_t;

typedef enum {
	MODE_FULL,	   /* memory run, then timed runs */
	MODE_MEMORY,   /* memory run only (independent rss check) */
	MODE_BASELINE, /* parse and touch the arrays, no allocator calls */
} run_mode_t;

typedef struct {
	run_mode_t mode;
	bool	   profile;		 /* hardware counters in the timed runs */
	bool	   check_heap;	 /* tmm_check() after every op */
	int		   runs;		 /* timed runs, at most */
	double	   budget;		 /* stop after this many timed seconds, 0 = never */
	size_t	   sample_every; /* tmm_heap_size() every n ops in the memory run */
	bool	   header;		 /* print the csv header */
} opts_t;

typedef struct {
	size_t		peak_live; /* most payload bytes live at once */
	size_t		peak_heap; /* most heap bytes the allocator held */
	long		rss_kb; /* peak rss growth during the memory run, -1 if n/a */
	long		reserved_kb; /* peak virtual size growth, -1 if n/a */
	tmm_stats_t stats;
	double		ops_per_sec;
	double		hw[HW_EVENTS]; /* per timed op, user space only, -1 if n/a */
} result_t;

/* hardware counters, opened once with -p, counting only inside the timed
 * loops. -1 is "not open": 0 would be stdin */
static int hw_fd[HW_EVENTS] = {-1, -1, -1, -1, -1, -1, -1};

/* clang-format off */
static void *map_alloc(size_t size);
static void *map_grow(void *p, size_t old_size, size_t new_size);
static void map_free(void *p, size_t size);
static int parse_trace(const char *path, trace_t *t);
static int push_op(trace_t *t, op_t op);
static void free_trace(trace_t *t);
static int memory_run(const trace_t *t, const opts_t *o, void **ptrs, uint32_t *sizes, result_t *r);
static int time_run(const trace_t *t, void **ptrs, double *secs, bool count);
static void hw_open(void);
static void hw_toggle(bool on);
static double hw_total(int i);
static void free_leftovers(const trace_t *t, void **ptrs);
static void fill_payload(uint8_t *p, size_t size, uint32_t id);
static bool payload_intact(const uint8_t *p, size_t size, uint32_t id);
static long status_kb(const char *key);
static int reset_hwm(void);
static double now_sec(void);
static int cmp_double(const void *a, const void *b);
static void print_row(const trace_t *t, const result_t *r);
static void usage(const char *prog);
/* clang-format on */

/*
 * the driver's own big arrays come straight from mmap. if they came from
 * malloc they would sit inside glibc's heap and mallinfo2() would count them
 * against glibc. mmap memory is zeroed, so these double as calloc.
 */
static void *
map_alloc(size_t size)
{
	void *p = mmap(
		NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	return p == MAP_FAILED ? NULL : p;
}

static void *
map_grow(void *p, size_t old_size, size_t new_size)
{
	if (!p)
		return map_alloc(new_size);
	void *q = mremap(p, old_size, new_size, MREMAP_MAYMOVE);
	return q == MAP_FAILED ? NULL : q;
}

static void
map_free(void *p, size_t size)
{
	if (p)
		munmap(p, size);
}

static int
push_op(trace_t *t, op_t op)
{
	if (t->n_ops == t->cap) {
		size_t cap = t->cap ? t->cap * 2 : 4096;
		op_t  *ops =
			(op_t *)map_grow(t->ops, t->cap * sizeof(op_t), cap * sizeof(op_t));
		if (!ops) {
			perror("driver: mremap");
			return -1;
		}
		t->ops = ops;
		t->cap = cap;
	}
	t->ops[t->n_ops++] = op;
	return 0;
}

static void
free_trace(trace_t *t)
{
	map_free(t->ops, t->cap * sizeof(op_t));
	*t = (trace_t){0};
}

/*
 * format, one op per line, '#' starts a comment:
 *	 a <id> <size>	 allocate size bytes under id
 *	 f <id>			 free the block allocated under id
 * the trace is checked while parsing: no double alloc, no free of a dead id.
 */
static int
parse_trace(const char *path, trace_t *t)
{
	*t		= (trace_t){0};
	t->path = path;

	FILE *f = fopen(path, "r");
	if (!f) {
		fprintf(stderr, "driver: %s: %s\n", path, strerror(errno));
		return -1;
	}

	char	*line	 = NULL;
	size_t	 len	 = 0;
	size_t	 lineno	 = 0;
	uint8_t *live	 = NULL; /* live[id] != 0 while id is allocated */
	size_t	 live_sz = 0;
	int		 ret	 = -1;

	while (getline(&line, &len, f) != -1) {
		lineno++;
		char *s = line;
		while (isspace((unsigned char)*s)) s++;
		if (*s == '\0' || *s == '#')
			continue;

		char		  kind = 0;
		unsigned long id   = 0;
		unsigned long size = 0;
		int			  n	   = sscanf(s, "%c %lu %lu", &kind, &id, &size);
		op_t		  op   = {0};

		if (kind == 'a' && n == 3 && size > 0 && size <= UINT32_MAX) {
			op.kind = OP_ALLOC;
		} else if (kind == 'f' && n >= 2) {
			op.kind = OP_FREE;
		} else {
			fprintf(stderr, "driver: %s:%zu: bad line\n", path, lineno);
			goto out;
		}
		if (id >= UINT32_MAX) {
			fprintf(stderr, "driver: %s:%zu: id too large\n", path, lineno);
			goto out;
		}
		op.id	= (uint32_t)id;
		op.size = (uint32_t)size;

		if (op.id >= live_sz) {
			size_t sz = live_sz ? live_sz : 1024;
			while (sz <= op.id) sz *= 2;
			uint8_t *l = (uint8_t *)map_grow(live, live_sz, sz);
			if (!l) {
				perror("driver: mremap");
				goto out;
			}
			live	= l;
			live_sz = sz;
		}

		if (op.kind == OP_ALLOC && live[op.id]) {
			fprintf(stderr,
					"driver: %s:%zu: id %u allocated twice\n",
					path,
					lineno,
					op.id);
			goto out;
		}
		if (op.kind == OP_FREE && !live[op.id]) {
			fprintf(stderr,
					"driver: %s:%zu: id %u freed but not live\n",
					path,
					lineno,
					op.id);
			goto out;
		}
		live[op.id] = op.kind == OP_ALLOC;

		if (push_op(t, op) != 0)
			goto out;
		if (op.id + 1 > t->n_ids)
			t->n_ids = op.id + 1;
	}

	if (t->n_ops == 0) {
		fprintf(stderr, "driver: %s: no ops\n", path);
		goto out;
	}
	ret = 0;

out:
	free(line);
	map_free(live, live_sz);
	fclose(f);
	if (ret != 0)
		free_trace(t);
	return ret;
}

/* every id gets its own 64-bit pattern, so overlapping blocks show up as a
 * corrupted payload when the first of them is freed */
static void
fill_payload(uint8_t *p, size_t size, uint32_t id)
{
	uint64_t pat = (id + 1) * 0x9e3779b97f4a7c15ULL;
	for (size_t i = 0; i < size; i++) p[i] = (uint8_t)(pat >> ((i & 7) << 3));
}

static bool
payload_intact(const uint8_t *p, size_t size, uint32_t id)
{
	uint64_t pat = (id + 1) * 0x9e3779b97f4a7c15ULL;
	for (size_t i = 0; i < size; i++) {
		if (p[i] != (uint8_t)(pat >> ((i & 7) << 3)))
			return false;
	}
	return true;
}

/* value of a "Key:   123 kB" line in /proc/self/status, -1 if missing.
 * plain open/read and a stack buffer: stdio would malloc, and with glibc
 * linked that lands in the very heap being measured */
static long
status_kb(const char *key)
{
	char buf[4096];
	int	 fd = open("/proc/self/status", O_RDONLY);
	if (fd < 0)
		return -1;
	ssize_t n = read(fd, buf, sizeof(buf) - 1);
	close(fd);
	if (n <= 0)
		return -1;
	buf[n]	   = '\0';

	char *line = strstr(buf, key);
	return line ? strtol(line + strlen(key), NULL, 10) : -1;
}

/* writing 5 to clear_refs resets VmHWM (peak rss) to the current rss
 * (Documentation/filesystems/proc.rst, linux 4.0+) */
static int
reset_hwm(void)
{
	int fd = open("/proc/self/clear_refs", O_WRONLY);
	if (fd < 0)
		return -1;
	int ret = write(fd, "5", 1) == 1 ? 0 : -1;
	close(fd);
	return ret;
}

/*
 * one untimed run that does all the checking and memory measurement: payload
 * patterns, alignment, tmm_check() with -c, peak live vs peak heap, rss.
 */
static int
memory_run(const trace_t *t,
		   const opts_t	 *o,
		   void			**ptrs,
		   uint32_t		 *sizes,
		   result_t		 *r)
{
	long vm_base = status_kb("VmSize:"); /* before tmm_init reserves anything */
	if (tmm_init() != 0) {
		fprintf(stderr, "driver: tmm_init failed\n");
		return -1;
	}
	r->reserved_kb = -1;
	/* touch both arrays first, so their pages are not counted as heap rss */
	memset(ptrs, 0, t->n_ids * sizeof(*ptrs));
	memset(sizes, 0, t->n_ids * sizeof(*sizes));

	long   rss_base = reset_hwm() == 0 ? status_kb("VmRSS:") : -1;
	size_t live		= 0;
	int	   ret		= -1;

	for (size_t i = 0; i < t->n_ops; i++) {
		const op_t *op = &t->ops[i];

		if (op->kind == OP_ALLOC) {
			uint8_t *p = (uint8_t *)tmm_malloc(op->size);
			if (!p) {
				fprintf(stderr,
						"driver: op %zu: tmm_malloc(%u) returned NULL\n",
						i,
						op->size);
				goto out;
			}
			if ((uintptr_t)p % TMM_ALIGNMENT != 0) {
				fprintf(stderr,
						"driver: op %zu: %p is not %d-byte aligned\n",
						i,
						(void *)p,
						TMM_ALIGNMENT);
				goto out;
			}
			fill_payload(p, op->size, op->id);
			ptrs[op->id]  = p;
			sizes[op->id] = op->size;
			live += op->size;
			if (live > r->peak_live)
				r->peak_live = live;
		} else {
			uint8_t *p = (uint8_t *)ptrs[op->id];
			if (!payload_intact(p, sizes[op->id], op->id)) {
				fprintf(stderr,
						"driver: op %zu: payload of id %u was overwritten\n",
						i,
						op->id);
				goto out;
			}
			tmm_free(p);
			ptrs[op->id] = NULL;
			live -= sizes[op->id];
		}

		if (o->check_heap && tmm_check() != 0) {
			fprintf(stderr, "driver: op %zu: tmm_check failed\n", i);
			goto out;
		}
		if (i % o->sample_every == 0 || i == t->n_ops - 1) {
			size_t heap = tmm_heap_size();
			if (heap > r->peak_heap) {
				r->peak_heap = heap;
				/* virtual size at each new heap peak, independent of the
				 * allocator's own accounting */
				long vm		 = status_kb("VmSize:");
				if (vm >= 0 && vm_base >= 0 && vm - vm_base > r->reserved_kb)
					r->reserved_kb = vm - vm_base;
			}
		}
	}

	long hwm  = rss_base >= 0 ? status_kb("VmHWM:") : -1;
	r->rss_kb = hwm >= 0 ? hwm - rss_base : -1;
	r->stats  = tmm_get_stats();
	ret		  = 0;

out:
	free_leftovers(t, ptrs);
	return ret;
}

static double
now_sec(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

/*
 * the same counters perf stat uses, through perf_event_open directly, so
 * they cover only the timed loop and not parsing or the memory run. user
 * space only: perf_event_paranoid 2 allows no more, so kernel work such as
 * glibc's mmap calls is not in these numbers. a counter that cannot be
 * opened stays -1 and is reported as n/a.
 */
static void
hw_open(void)
{
	static const struct {
		uint32_t type;
		uint64_t config;
	} ev[HW_EVENTS] = {
		{PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES	},
		{PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS },
		{PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES},
		{PERF_TYPE_HW_CACHE,
		 PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
			 (PERF_COUNT_HW_CACHE_RESULT_MISS << 16)	},
		{PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES },
		{PERF_TYPE_HW_CACHE,
		 PERF_COUNT_HW_CACHE_DTLB | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
			 (PERF_COUNT_HW_CACHE_RESULT_MISS << 16)	},
		{PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS	},
	};
	/* no llc-load-misses: perf has no generic llc event on this cpu, so the
	 * generic cache-misses event stands in, as perf stat itself does */

	for (int i = 0; i < HW_EVENTS; i++) {
		struct perf_event_attr a = {0};
		a.size					 = sizeof(a);
		a.type					 = ev[i].type;
		a.config				 = ev[i].config;
		a.disabled				 = 1;
		a.exclude_kernel		 = 1;
		a.exclude_hv			 = 1;
		a.read_format =
			PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;
		hw_fd[i] = (int)syscall(SYS_perf_event_open, &a, 0, -1, -1, 0);
	}
}

static void
hw_toggle(bool on)
{
	for (int i = 0; i < HW_EVENTS; i++) {
		if (hw_fd[i] >= 0)
			ioctl(hw_fd[i],
				  on ? PERF_EVENT_IOC_ENABLE : PERF_EVENT_IOC_DISABLE,
				  0);
	}
}

/* total over every enabled window, scaled up if the kernel had to share the
 * hardware counter with other events */
static double
hw_total(int i)
{
	uint64_t v[3]; /* value, time enabled, time running */
	if (hw_fd[i] < 0 || read(hw_fd[i], v, sizeof(v)) != sizeof(v) || v[2] == 0)
		return -1;
	return (double)v[0] * ((double)v[1] / (double)v[2]);
}

/* the timed loop does nothing but call the allocator */
static int
time_run(const trace_t *t, void **ptrs, double *secs, bool count)
{
	if (tmm_init() != 0)
		return -1;
	memset(ptrs, 0, t->n_ids * sizeof(*ptrs));

	if (count)
		hw_toggle(true);
	double start = now_sec();
	for (size_t i = 0; i < t->n_ops; i++) {
		const op_t *op = &t->ops[i];
		if (op->kind == OP_ALLOC) {
			ptrs[op->id] = tmm_malloc(op->size);
		} else {
			tmm_free(ptrs[op->id]);
			ptrs[op->id] = NULL;
		}
	}
	*secs = now_sec() - start;
	if (count)
		hw_toggle(false);

	free_leftovers(t, ptrs);
	return 0;
}

/* glibc keeps its heap between runs, so whatever a trace leaves live is freed
 * outside the timed region */
static void
free_leftovers(const trace_t *t, void **ptrs)
{
	for (uint32_t id = 0; id < t->n_ids; id++) {
		if (ptrs[id]) {
			tmm_free(ptrs[id]);
			ptrs[id] = NULL;
		}
	}
}

static int
cmp_double(const void *a, const void *b)
{
	double x = *(const double *)a;
	double y = *(const double *)b;
	return (x > y) - (x < y);
}

static void
print_row(const trace_t *t, const result_t *r)
{
	/* trace name without directory and extension */
	const char *slash = strrchr(t->path, '/');
	const char *name  = slash ? slash + 1 : t->path;
	const char *dot	  = strrchr(name, '.');
	int			nlen  = dot ? (int)(dot - name) : (int)strlen(name);
	double		util =
		r->peak_heap ? (double)r->peak_live / (double)r->peak_heap : 0.0;

	printf(
		"%s,%.*s,%zu,%.0f,%zu,%zu,%.4f,%ld,%lu,%lu,%lu,%lu,%lu,%ld,%.2f,%.2f,"
		"%.4f,%.4f,%.4f,%.4f,%.4f\n",
		tmm_name(),
		nlen,
		name,
		t->n_ops,
		r->ops_per_sec,
		r->peak_live,
		r->peak_heap,
		util,
		r->rss_kb,
		r->stats.sbrk_calls,
		r->stats.syscalls,
		r->stats.blocks_scanned,
		r->stats.splits,
		r->stats.coalesces,
		r->reserved_kb,
		r->hw[0],
		r->hw[1],
		r->hw[2],
		r->hw[3],
		r->hw[4],
		r->hw[5],
		r->hw[6]);
}

static void
usage(const char *prog)
{
	fprintf(stderr,
			"usage: %s [-c] [-p] [-M | -B] [-n runs] [-s every] [-H] [trace]\n"
			"  -c        run tmm_check() after every op (slow)\n"
			"  -p        profile: hardware counters around the timed runs\n"
			"  -M        memory run only, no timing (for the rss check)\n"
			"  -B        baseline: parse and touch arrays, no allocator\n"
			"  -N        no transparent huge pages in this process\n"
			"  -n runs   timed runs, median is reported (default %d)\n"
			"  -T secs   stop early once this much is timed (3 runs min)\n"
			"  -s every  sample heap size every n ops (default 1)\n"
			"  -H        print the csv header\n"
			"one trace per process: glibc keeps its heap between traces\n",
			prog,
			DEFAULT_RUNS);
}

int
main(int argc, char **argv)
{
	opts_t o = {
		.mode		  = MODE_FULL,
		.profile	  = false,
		.check_heap	  = false,
		.runs		  = DEFAULT_RUNS,
		.budget		  = 0,
		.sample_every = 1,
		.header		  = false,
	};

	int c;
	while ((c = getopt(argc, argv, "cpMBNn:T:s:Hh")) != -1) {
		switch (c) {
		case 'c': o.check_heap = true; break;
		case 'p': o.profile = true; break;
		case 'M': o.mode = MODE_MEMORY; break;
		case 'B': o.mode = MODE_BASELINE; break;
		case 'N':
			/* no transparent huge pages for anything this process maps,
			 * glibc's heap included, so page size can't vary per run */
			if (prctl(PR_SET_THP_DISABLE, 1, 0, 0, 0) != 0)
				perror("driver: prctl thp");
			break;
		case 'n': o.runs = atoi(optarg); break;
		case 'T': o.budget = atof(optarg); break;
		case 's': o.sample_every = strtoul(optarg, NULL, 10); break;
		case 'H': o.header = true; break;
		case 'h': usage(argv[0]); return 0;
		default: usage(argv[0]); return 2;
		}
	}

	if (o.runs < 1 || o.sample_every < 1) {
		usage(argv[0]);
		return 2;
	}
	if (o.header)
		puts(CSV_HEADER);
	if (optind == argc && o.header)
		return 0;
	if (argc - optind != 1) {
		usage(argv[0]);
		return 2;
	}

	trace_t t;
	if (parse_trace(argv[optind], &t) != 0)
		return 1;

	int		 ret = 1;
	result_t r	 = {.rss_kb = -1, .reserved_kb = -1};
	for (int i = 0; i < HW_EVENTS; i++) r.hw[i] = -1;
	void	**ptrs	= (void **)map_alloc(t.n_ids * sizeof(void *));
	uint32_t *sizes = (uint32_t *)map_alloc(t.n_ids * sizeof(uint32_t));
	double	 *secs	= (double *)calloc((size_t)o.runs, sizeof(double));
	if (!ptrs || !sizes || !secs) {
		perror("driver: alloc");
		goto out;
	}

	if (o.mode == MODE_BASELINE) {
		/* the same footprint as a real run, minus the allocator: what the
		 * independent rss check subtracts */
		memset(ptrs, 0, t.n_ids * sizeof(*ptrs));
		memset(sizes, 0, t.n_ids * sizeof(*sizes));
		ret = 0;
		goto out;
	}

	if (memory_run(&t, &o, ptrs, sizes, &r) != 0)
		goto out;
	if (o.mode == MODE_MEMORY) {
		print_row(&t, &r);
		ret = 0;
		goto out;
	}

	if (o.profile)
		hw_open();
	double warmup;
	if (time_run(&t, ptrs, &warmup, false) != 0)
		goto out;
	/* a long trace doesn't need ten copies to beat noise: with a budget,
	 * stop once that much is timed, but never before 3 runs */
	int	   done	 = 0;
	double timed = 0;
	while (done < o.runs) {
		if (time_run(&t, ptrs, &secs[done], o.profile) != 0)
			goto out;
		timed += secs[done++];
		if (o.budget > 0 && done >= 3 && timed >= o.budget)
			break;
	}
	for (int i = 0; i < HW_EVENTS && o.profile; i++) {
		double total = hw_total(i);
		r.hw[i]		 = total < 0 ? -1 : total / ((double)t.n_ops * done);
	}
	qsort(secs, (size_t)done, sizeof(double), cmp_double);
	double median =
		done % 2 ? secs[done / 2] : (secs[done / 2 - 1] + secs[done / 2]) / 2;
	r.ops_per_sec = (double)t.n_ops / median;

	print_row(&t, &r);
	ret = 0;

out:
	free(secs);
	map_free(sizes, t.n_ids * sizeof(uint32_t));
	map_free(ptrs, t.n_ids * sizeof(void *));
	free_trace(&t);
	return ret;
}
