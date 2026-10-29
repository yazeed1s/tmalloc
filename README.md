# tmalloc

tmalloc is my own version of `malloc` and `free`, written in C for a class project (CS 5204).

When a C program calls `malloc(100)`, something has to find 100 free bytes in memory and remember that they are now in use. When the program calls `free`, that space has to be given back so it can be reused. That "something" is the memory allocator. Normally it comes from the C library (glibc on Linux). In this project I wrote my own, in a few versions, and compared them against glibc to see where mine is faster, where it is slower, and why.

## The versions

Each version is one file in `src/`, and each one fixes the main problem of the one before it:

| Version | File | How it finds a free block |
|---|---|---|
| implicit | `src/implicit.c` | Walks every block in the heap, used or free, until one fits. Simple but slow. |
| explicit | `src/explicit.c` | Keeps the free blocks in their own linked list, so it only looks at free ones. |
| segregated | `src/segregated.c` | Keeps several free lists, one per size range, so it starts looking near the right size. |
| segregated-skip | `src/segregated.c` (built with `-DSEG_SKIP_OWN_CLASS`) | Same, but always jumps to a bigger size range, so it never has to search. |
| glibc | `src/glibc.c` | Just calls the normal `malloc`/`free`, so the comparison has a baseline. |

All versions share the same interface (`src/tmalloc.h`): `tmm_malloc`, `tmm_free`, plus a heap checker and some counters. The functions are prefixed with `tmm_` so they don't replace the real `malloc` that the rest of the program still uses.

The heap memory itself comes from `src/memlib.c`. It reserves a big range of addresses once and grows the heap a piece at a time, like the old Unix `sbrk`.

## Quick start

You need Linux, `gcc`, `make` and `python3`.

```sh
./setup.sh      # generate the traces, run the tests, build everything
make bench      # run every version on every trace, writes a CSV to results/
```

Other useful commands:

```sh
make test       # heap-checker tests for every version
make asan       # build with AddressSanitizer
make profile    # same as bench, but also records CPU counters (cache misses etc.)
make plots      # turn the results into the figures in results/figures/
```

To run one version on one trace yourself:

```sh
./bench_explicit traces/tiny-basic.trace        # prints one CSV line
./bench_explicit -c traces/tiny-basic.trace     # also checks the heap after every operation
```

Run `./bench_explicit -h` to see all options.

## How the benchmark works

A **trace** is a small text file that lists `malloc` and `free` calls in order:

```
a 0 24     # allocate 24 bytes, call it block 0
a 1 100    # allocate 100 bytes, call it block 1
f 1        # free block 1
```

`tools/gentrace.py` generates the traces into `traces/gen/` (they are seeded, so you get the same files every time). The benchmark program, `bench/driver.c`, replays a trace against one allocator, times it, and reports speed, memory use, and how many blocks the allocator had to search. The same driver is built once per version (`bench_implicit`, `bench_explicit`, ...), so every allocator runs the exact same code around it.

Timings on a laptop jump around a lot. `quiet-bench.sh` fixes the CPU frequency, pins the benchmark to one core and runs everything three times. The numbers in the report come from that script.

## What I found (short version)

- The implicit list is very slow, as expected (36x to 1350x slower than glibc).
- The explicit list is about as fast as glibc on average, and faster when the program has a moderate number of blocks of mixed sizes.
- glibc wins clearly when there are very many live blocks, when the program uses mostly small objects of a few sizes, and when there are many large free blocks. In each case my allocator ends up searching or merging blocks that glibc avoids.
- Mine also uses more memory than glibc once the heap gets large.

The full write-up, with figures, is in `midterm.tex` / `midterm.pdf`.

## Folder layout

```
src/        the allocators, memlib, and the shared header
bench/      the benchmark driver
tests/      tests for memlib and the heap checker
tools/      trace generator, benchmark runner, plotting scripts
traces/     hand-written tiny traces, plus generated ones in traces/gen/
results/    benchmark CSVs and figures
midterm.tex the midterm report
plan.md     my working plan for the midterm
```

## What's next

The allocators are single-threaded for now. The next steps are finer size classes, a sorted list for large blocks, and per-thread caches so it works well with multiple threads. See the last section of the report for the schedule.

## License

BSD 2-Clause, see the header of each source file.
