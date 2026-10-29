#!/usr/bin/env python3
"""merge the quiet-bench rounds into one csv and a markdown table.

usage: summarize.py results/midterm-1.csv results/midterm-2.csv ...

writes results/midterm.csv (ops_per_sec = median over rounds, plus the
max/min spread) and results/midterm.md (the table the report uses). the
memory and counter columns are deterministic, so they must be identical in
every round; if they are not, something changed between rounds and the
script stops.
"""

import csv
import platform
import statistics
import subprocess
import sys

DETERMINISTIC = ("ops", "peak_live", "peak_heap", "peak_util", "sbrk_calls",
                 "syscalls", "blocks_scanned", "splits", "coalesces")
VERSIONS = ("glibc", "implicit", "explicit", "segregated", "segregated-skip")
HW = ("instr_per_op", "cache_miss_per_op", "dtlb_miss_per_op")


def machine():
    """the details the report's methodology section needs"""
    cpu = next((l.split(":", 1)[1].strip() for l in open("/proc/cpuinfo")
                if l.startswith("model name")), "unknown cpu")
    first = lambda cmd: subprocess.run(cmd, capture_output=True,
                                       text=True).stdout.splitlines()[0]
    return (f"{cpu}, linux {platform.release()}, {first(['ldd', '--version'])}, "
            f"{first(['gcc', '--version'])}, -O2, pinned with taskset, "
            f"performance governor, boost off")


FAMILIES = (
    ("main suite", "main"),
    ("scaling (exact live set)", "scale"),
    ("holes, large holes, huge blocks", "stress"),
    ("exact-size recycling (pinned vs freed separators)", "recycle"),
    ("fragmentation", "frag"),
    ("size distributions", "dist"),
)


def family(trace):
    for prefix in ("scale", "recycle", "frag", "dist"):
        if trace.startswith(prefix + "-"):
            return prefix
    if trace.startswith(("holes-", "large-holes-", "huge-")):
        return "stress"
    return "main"


def profile_table(f):
    """the separate profiling pass (results/profile.csv), per op"""
    try:
        rows = list(csv.DictReader(open("results/profile.csv")))
    except FileNotFoundError:
        return
    f.write("\n## hardware counters (profiling pass, user space, per op)\n\n")
    f.write("| trace | version | cycles | instr | IPC | branch miss | L1D miss | "
            "cache miss | dTLB miss | page faults |\n")
    f.write("|---|---|---|---|---|---|---|---|---|---|\n")
    for r in rows:
        if r["trace"].startswith("tiny") or float(r["cycles_per_op"]) < 0:
            continue
        cyc, ins = float(r["cycles_per_op"]), float(r["instr_per_op"])
        f.write(f"| {r['trace']} | {r['version']} | {cyc:.0f} | {ins:.0f} | "
                f"{ins / cyc:.2f} | {float(r['branch_miss_per_op']):.2f} | "
                f"{float(r['l1d_miss_per_op']):.2f} | "
                f"{float(r['cache_miss_per_op']):.2f} | "
                f"{float(r['dtlb_miss_per_op']):.2f} | "
                f"{float(r['page_faults_per_op']):.4f} |\n")


def rss_table(f):
    """the independent rss check (results/rss-check.csv)"""
    try:
        rows = list(csv.DictReader(open("results/rss-check.csv")))
    except FileNotFoundError:
        return
    f.write("\n## memory: live, logical heap, reserved, resident\n\n")
    f.write("driver rss = clear_refs/VmHWM inside the process; independent = "
            "child maxrss (wait4) minus a no-allocator baseline run\n\n")
    f.write("| trace | version | peak live MB | heap MB | reserved MB | "
            "rss (driver) MB | rss (independent) MB |\n")
    f.write("|---|---|---|---|---|---|---|\n")
    for r in rows:
        mb = lambda k: f"{int(r[k]) / 1024:.1f}"  # noqa: E731
        f.write(f"| {r['trace']} | {r['version']} | {mb('peak_live_kb')} | "
                f"{mb('heap_kb')} | {mb('reserved_kb')} | "
                f"{mb('driver_rss_kb')} | {mb('indep_rss_kb')} |\n")


def main():
    paths = sys.argv[1:]
    if len(paths) < 2:
        sys.exit("usage: summarize.py round1.csv round2.csv ...")
    rounds = [list(csv.DictReader(open(p))) for p in paths]
    keyed = [{(r["version"], r["trace"]): r for r in rows} for rows in rounds]

    merged = []
    for row in rounds[0]:
        key = (row["version"], row["trace"])
        others = [k[key] for k in keyed]
        for col in DETERMINISTIC:
            vals = {o[col] for o in others}
            if len(vals) != 1:
                sys.exit(f"summarize: {key} {col} differs between rounds: {vals}")
        speeds = [float(o["ops_per_sec"]) for o in others]
        out = dict(row)
        out["ops_per_sec"] = f"{statistics.median(speeds):.0f}"
        out["ops_spread"] = f"{max(speeds) / min(speeds):.3f}"
        for col in HW:  # counters from the timed loop, a bit noisy too
            if col in row:
                out[col] = f"{statistics.median(float(o[col]) for o in others):.4f}"
        merged.append(out)

    fields = list(rounds[0][0].keys()) + ["ops_spread"]
    with open("results/midterm.csv", "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        w.writerows(merged)

    by = {(r["version"], r["trace"]): r for r in merged}
    traces = [r["trace"] for r in merged
              if r["version"] == "glibc" and not r["trace"].startswith("tiny")]
    with open("results/midterm.md", "w") as f:
        f.write(f"machine: {machine()}\n\n")
        f.write(f"median of {len(paths)} rounds, 10 timed runs each\n")
        for title, prefix in FAMILIES:
            group = [t for t in traces if family(t) == prefix]
            if not group:
                continue
            f.write(f"\n## {title}\n\n")
            f.write("| trace | version | Mops/s | vs glibc | scans/malloc | "
                    "splits/malloc | peak util | rss MB |\n")
            f.write("|---|---|---|---|---|---|---|---|\n")
            for t in group:
                g = float(by[("glibc", t)]["ops_per_sec"])
                for v in VERSIONS:
                    if (v, t) not in by:
                        continue  # implicit skips the biggest scale traces
                    r = by[(v, t)]
                    ops = float(r["ops_per_sec"])
                    mallocs = int(r["ops"]) // 2  # every trace frees all it allocates
                    zm = v != "glibc"
                    scans = f"{int(r['blocks_scanned']) / mallocs:.1f}" if zm else "-"
                    splits = f"{int(r['splits']) / mallocs:.2f}" if zm else "-"
                    f.write(f"| {t} | {v} | {ops / 1e6:.2f} | {ops / g:.3f}x | "
                            f"{scans} | {splits} | {float(r['peak_util']):.3f} | "
                            f"{int(r['rss_kb']) / 1024:.1f} |\n")
            f.write("\nrankings (fastest first):\n\n")
            for t in group:
                order = sorted((v for v in VERSIONS if (v, t) in by),
                               key=lambda v: -float(by[(v, t)]["ops_per_sec"]))
                f.write(f"- {t}: {' > '.join(order)}\n")
        profile_table(f)
        rss_table(f)
    worst = max(merged, key=lambda r: float(r["ops_spread"])
                if not r["trace"].startswith("tiny") else 0)
    print(f"wrote results/midterm.csv and results/midterm.md "
          f"(worst spread {worst['ops_spread']}x: {worst['version']} {worst['trace']})")


if __name__ == "__main__":
    main()
