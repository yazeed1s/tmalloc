#!/usr/bin/env python3
"""independent check of the driver's rss number.

usage: rsscheck.py bench_X... > results/rss-check.csv

the driver measures resident memory from inside the process: it resets the
peak-rss counter through /proc/self/clear_refs right before the memory run
and reads VmHWM after it. this script measures the same thing from outside,
without trusting that: every run is its own child process, and the kernel
reports the child's lifetime peak rss through wait4. two children per
allocator and trace:

  -M  the real memory run
  -B  the same trace parsed and the same arrays touched, no allocator calls

independent rss = maxrss(-M) - maxrss(-B). if the clear_refs method is
right, the two agree up to the few pages the driver itself touches.
"""

import csv
import io
import subprocess
import sys

TRACES = ("small-fixed", "random", "bimodal", "throughput", "grow-free-all",
          "checkerboard", "scale-1k", "scale-4k", "scale-16k", "scale-64k",
          "scale-128k", "huge-churn", "large-holes-2000", "dist-loguniform")
SKIP = {("bench_implicit", f"scale-{k}k") for k in (8, 16, 32, 64, 128)}


def run(args):
    """run a child through tools/maxrss, return its stdout and its lifetime
    peak rss in KiB. forking the driver from python directly would put
    python's own rss under every number (the kernel keeps a forked child's
    starting rss as its peak across exec), so the tiny launcher does it"""
    p = subprocess.run(["./tools/maxrss"] + args, capture_output=True,
                       text=True)
    if p.returncode != 0:
        sys.exit(f"rsscheck: {' '.join(args)} failed: {p.stderr}")
    peak = next(int(l.split()[1]) for l in p.stderr.splitlines()
                if l.startswith("maxrss_kb"))
    return p.stdout, peak


def main():
    bins = sys.argv[1:]
    w = csv.writer(sys.stdout)
    w.writerow(["version", "trace", "peak_live_kb", "heap_kb", "reserved_kb",
                "driver_rss_kb", "indep_rss_kb", "baseline_kb"])
    for b in bins:
        for t in TRACES:
            if (b, t) in SKIP:
                continue
            path = f"traces/gen/{t}.trace"
            out, full = run([f"./{b}", "-M", path])
            _, base = run([f"./{b}", "-B", path])
            row = next(csv.reader(io.StringIO(out)))
            # columns as in the driver's csv header
            w.writerow([row[0], row[1], int(row[4]) // 1024,
                        int(row[5]) // 1024, row[13], row[7],
                        full - base, base])
            sys.stdout.flush()


if __name__ == "__main__":
    main()
