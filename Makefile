CC = gcc
CFLAGS = -std=gnu11 -Wall -Wextra -Wshadow -Wunreachable-code -Wcast-align \
         -Wuninitialized -Isrc
LDFLAGS =

SRC_DIR = ./src
BENCH_DIR = ./bench
TESTS_DIR = ./tests
DRIVER = $(BENCH_DIR)/driver.c
MEMLIB = $(SRC_DIR)/memlib.c
HEADER_FILES = $(SRC_DIR)/tmalloc.h $(SRC_DIR)/memlib.h

# a version is built only once its source file exists
VERSIONS = implicit explicit segregated glibc
BUILT = $(foreach v,$(VERSIONS),$(if $(wildcard $(SRC_DIR)/$(v).c),$(v)))
# fit-policy variant of segregated, see find_fit() in segregated.c (Q2 b)
VARIANTS = $(if $(filter segregated,$(BUILT)),segregated-skip)
BINS = $(addprefix bench_,$(BUILT) $(VARIANTS))
CHECK_TESTS = $(addprefix check_test_,$(filter-out glibc,$(BUILT)) $(VARIANTS))

DEBUG_FLAGS = -g -O0 -D_DEBUG__=1
ASAN_FLAGS = -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer -g -O0

.DEFAULT_GOAL := release

release: CFLAGS += -O2 -DNDEBUG
release: clean $(BINS)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean $(BINS)

asan: CFLAGS += $(ASAN_FLAGS)
asan: LDFLAGS += $(ASAN_FLAGS)
asan: clean $(BINS)

# glibc does not need memlib, everything else grows its heap through it
bench_glibc: $(DRIVER) $(SRC_DIR)/glibc.c $(HEADER_FILES)
	$(CC) $(CFLAGS) -o $@ $(DRIVER) $(SRC_DIR)/glibc.c $(LDFLAGS)

bench_segregated-skip: $(DRIVER) $(SRC_DIR)/segregated.c $(MEMLIB) $(HEADER_FILES)
	$(CC) $(CFLAGS) -DSEG_SKIP_OWN_CLASS -o $@ $(DRIVER) $(SRC_DIR)/segregated.c $(MEMLIB) $(LDFLAGS)

bench_%: $(DRIVER) $(SRC_DIR)/%.c $(MEMLIB) $(HEADER_FILES)
	$(CC) $(CFLAGS) -o $@ $(DRIVER) $(SRC_DIR)/$*.c $(MEMLIB) $(LDFLAGS)

memlib_test: $(TESTS_DIR)/memlib_test.c $(MEMLIB) $(SRC_DIR)/memlib.h
	$(CC) $(CFLAGS) -g -o $@ $(TESTS_DIR)/memlib_test.c $(MEMLIB)

# corrupts the heap on purpose, one binary per real allocator
check_test_segregated-skip: $(TESTS_DIR)/check_test.c $(SRC_DIR)/segregated.c $(MEMLIB) $(HEADER_FILES)
	$(CC) $(CFLAGS) -DSEG_SKIP_OWN_CLASS -g -o $@ $(TESTS_DIR)/check_test.c $(SRC_DIR)/segregated.c $(MEMLIB)

check_test_%: $(TESTS_DIR)/check_test.c $(SRC_DIR)/%.c $(MEMLIB) $(HEADER_FILES)
	$(CC) $(CFLAGS) -g -o $@ $(TESTS_DIR)/check_test.c $(SRC_DIR)/$*.c $(MEMLIB)

test: memlib_test $(CHECK_TESTS)
	./memlib_test
	@for t in $(CHECK_TESTS); do ./$$t || exit 1; done

traces:
	python3 tools/gentrace.py traces/gen

bench: release traces
	./tools/bench.sh $(BINS)

# hardware counters, a separate pass so throughput runs carry none
profile: release traces
	FLAGS=-p RUNS=5 OUT=results/profile.csv ./tools/bench.sh $(BINS)

# rss measured from outside the process, against the driver's own number
rsscheck: release traces tools/maxrss
	python3 tools/rsscheck.py $(BINS) > results/rss-check.csv

# launcher for the independent rss check (tools/rsscheck.py)
tools/maxrss: tools/maxrss.c
	$(CC) $(CFLAGS) -O2 -o $@ tools/maxrss.c

# merge the quiet-bench rounds and draw the figures (matplotlib in .venv)
plots:
	python3 tools/summarize.py results/midterm-[0-9].csv
	@test -x .venv/bin/python || { python3 -m venv .venv && .venv/bin/pip install -q matplotlib; }
	.venv/bin/python tools/plot.py

format:
	clang-format -i $(SRC_DIR)/*.c $(SRC_DIR)/*.h $(BENCH_DIR)/*.c $(TESTS_DIR)/*.c tools/*.c

clean:
	rm -f bench_* memlib_test check_test_* tools/maxrss

.PHONY: release debug asan test traces bench profile rsscheck plots format clean
