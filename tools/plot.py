#!/usr/bin/env python3
"""the midterm figures from results/midterm.csv.

usage: .venv/bin/python tools/plot.py   (needs matplotlib, see `make plots`)

  tradeoff.pdf  one point per allocator: mean utilization vs speed relative
                to glibc (geometric mean over the traces). the whole story.
  speed.pdf     one small bar chart per trace, Mops/s, linear, labeled
  util.pdf      the same layout for peak utilization
  scaling.pdf   scale-1k .. 128k: speed, blocks visited, splits, resident
                memory and utilization as the live set grows
  hardware.pdf  the same traces through cpu counters from the separate
                profiling pass (results/profile.csv), glibc included
  recycle.pdf   exact-size recycling, neighbours pinned vs freed
  dist-speed.pdf, dist-util.pdf   the size-distribution traces

every allocator keeps the same color in every figure, glibc is neutral gray.
the four hues pass the colorblind check all-pairs; every bar and point also
carries its name, so nothing depends on color alone.
"""

import csv
import math
import os

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

OUT = "results/figures"
TRACES = (  # trace, what it stresses
    ("small-fixed", "16-64 B, ~1000 live"),
    ("random", "1 B-4 KiB, random frees"),
    ("bimodal", "90% tiny, 10% large"),
    ("throughput", "1M ops, 16 B-1 KiB"),
    ("grow-free-all", "alloc 5000, free all"),
    ("checkerboard", "holes too small to reuse"),
)
ALLOCS = (  # version, label, color
    ("glibc", "glibc", "#8a8a85"),
    ("implicit", "implicit", "#2a78d6"),
    ("explicit", "explicit", "#eda100"),
    ("segregated", "segregated", "#e87ba4"),
    ("segregated-skip", "seg-skip", "#008300"),
)
INK = "#2b2b29"
MUTED = "#6b6b66"
GRID = "#e4e4e0"


def load():
    with open("results/midterm.csv") as f:
        return {(r["version"], r["trace"]): r for r in csv.DictReader(f)}


def clean(ax):
    for side in ("top", "right", "left"):
        ax.spines[side].set_visible(False)
    ax.spines["bottom"].set_color(GRID)
    ax.tick_params(axis="x", labelsize=7.5, colors=MUTED, length=0)
    ax.tick_params(axis="y", length=0)
    ax.grid(axis="x", color=GRID, linewidth=0.8)
    ax.set_axisbelow(True)


def tradeoff(data):
    fig, ax = plt.subplots(figsize=(6.5, 4.2))
    names = [t for t, _ in TRACES]
    # label offsets in points, picked so no two labels collide
    # (dx, dy, ha, va) in points, picked so no two labels collide.
    # glibc and explicit share a utilization, so they go left and right
    nudge = {"glibc": (-12, 0, "right", "center"),
             "explicit": (12, 0, "left", "center"),
             "implicit": (0, 10, "center", "bottom"),
             "segregated": (0, -17, "center", "top"),
             "segregated-skip": (0, -17, "center", "top")}
    for ver, label, color in ALLOCS:
        speeds = [float(data[(ver, t)]["ops_per_sec"])
                  / float(data[("glibc", t)]["ops_per_sec"]) for t in names]
        x = sum(float(data[(ver, t)]["peak_util"]) for t in names) / len(names)
        y = math.exp(sum(map(math.log, speeds)) / len(speeds))
        ax.plot(x, y, "o", markersize=11, color=color, markeredgecolor="white",
                markeredgewidth=1.5, zorder=3)
        ax.annotate(f"{label}\n{y:.2f}× speed, {x:.0%} util"
                    if ver != "glibc" else f"glibc\n1× speed, {x:.0%} util",
                    (x, y), textcoords="offset points",
                    xytext=nudge[ver][:2], ha=nudge[ver][2], va=nudge[ver][3],
                    fontsize=8, color=INK, linespacing=1.3)

    ax.set_xlim(0.6, 0.9)
    ax.set_ylim(-0.1, 1.25)
    ax.xaxis.set_major_formatter(plt.FuncFormatter(lambda v, _: f"{v:.0%}"))
    ax.yaxis.set_major_formatter(plt.FuncFormatter(lambda v, _: f"{v:g}×"))
    ax.set_xlabel("memory: mean peak utilization over the 6 traces  →  better",
                  fontsize=8.5, color=MUTED)
    ax.set_ylabel("speed relative to glibc (geometric mean)  →  better",
                  fontsize=8.5, color=MUTED)
    ax.set_title("Speed vs memory over the main suite", loc="left",
                 fontsize=10.5, color=INK, pad=10)
    clean(ax)
    ax.spines["left"].set_visible(True)
    ax.spines["left"].set_color(GRID)
    ax.tick_params(axis="y", labelsize=7.5, colors=MUTED)
    ax.grid(axis="y", color=GRID, linewidth=0.8)
    fig.subplots_adjust(left=0.12, right=0.97, top=0.9, bottom=0.13)
    return fig


def per_trace(data, value, fmt, title, xlim=None, traces=TRACES):
    fig, axes = plt.subplots(2, 3, figsize=(6.5, 5.0))
    for ax, (trace, what) in zip(axes.flat, traces):
        vals = [value(data, ver, trace) for ver, _, _ in ALLOCS]
        rows = range(len(ALLOCS))
        ax.barh(rows, vals, height=0.72, color=[c for _, _, c in ALLOCS],
                zorder=2)
        top = xlim if xlim else max(vals) * 1.32  # room for the labels
        for row, v in zip(rows, vals):
            ax.text(v + top * 0.02, row, fmt(v), va="center", fontsize=7,
                    color=INK)
        ax.set_xlim(0, top)
        ax.set_yticks(list(rows))
        ax.set_yticklabels([l for _, l, _ in ALLOCS], fontsize=7.5, color=INK)
        ax.invert_yaxis()
        ax.set_title(f"{trace}\n", loc="left", fontsize=8.5, color=INK,
                     fontweight="bold", pad=2)
        ax.text(0, 1.035, what, transform=ax.transAxes, fontsize=7,
                color=MUTED)
        clean(ax)
        ax.set_xticks([])
        ax.grid(False)
    fig.suptitle(title, x=0.02, ha="left", fontsize=10.5, color=INK)
    fig.subplots_adjust(left=0.125, right=0.98, top=0.86, bottom=0.03,
                        wspace=0.55, hspace=0.45)
    return fig


SCALE = tuple(f"scale-{k}k" for k in (1, 2, 4, 8, 16, 32, 64, 128))
SCALE_X = ("1k", "2k", "4k", "8k", "16k", "32k", "64k", "128k")
RECYCLE = tuple(str(n) for n in (32, 64, 128, 256, 512, 1024)) + ("mixed",)
DIST = (
    ("dist-small", "90% 8-64 B"),
    ("dist-bimodal", "70% 16-128 B, 30% 1-4 KiB"),
    ("dist-loguniform", "16 B-16 KiB, log-uniform"),
    ("dist-classes", "12 common fixed sizes"),
    ("dist-boundary", "edges of seg's classes"),
    ("frag-grow", "growing sizes, stranded holes"),
)
ZM = [a for a in ALLOCS if a[0] != "implicit"]  # implicit skips big traces


def lines(data, traces, xlabels, xlabel, panels, title, ncols=3):
    """one small line chart per panel, x = traces in order, end values
    labeled and pushed apart. panel = (title, value(row) or None to skip a
    version, label format, include glibc)"""
    rows = math.ceil(len(panels) / ncols)
    fig, axes = plt.subplots(rows, ncols, figsize=(6.5, 2.75 * rows + 0.5))
    axes = list(axes.flat) if rows > 1 or ncols > 1 else [axes]
    xs = range(len(traces))
    for ax, panel in zip(axes, panels):
        ptitle, value, fmt, with_glibc = panel[:4]
        log = len(panel) > 4 and panel[4] == "log"
        ends = []
        for ver, label, color in ZM:
            if ver == "glibc" and not with_glibc:
                continue
            ys = [value(data[(ver, t)]) for t in traces]
            ax.plot(xs, ys, color=color, linewidth=2, marker="o",
                    markersize=4.5, markeredgecolor="white",
                    markeredgewidth=1, label=label,
                    zorder=3 if ver != "glibc" else 2)
            ends.append(ys[-1])
        end_labels(ax, xs, ends, fmt, log)
        ax.set_xticks(list(xs))
        ax.set_xticklabels(xlabels, fontsize=6, rotation=45)
        ax.set_xlim(-0.3, len(traces) - 0.2)
        ax.set_xlabel(xlabel, fontsize=7, color=MUTED)
        ax.set_title(ptitle, loc="left", fontsize=8, color=INK, pad=5)
        clean(ax)
        ax.spines["left"].set_visible(True)
        ax.spines["left"].set_color(GRID)
        ax.tick_params(axis="y", labelsize=6.5, colors=MUTED, length=0)
        ax.tick_params(axis="x", colors=MUTED)
        ax.grid(axis="y", color=GRID, linewidth=0.8)
        ax.grid(axis="x", visible=False)
    for ax in axes[len(panels):]:
        ax.set_visible(False)
    handles, labels = [], []
    for ax in axes[:len(panels)]:  # union, in allocator order
        for h, l in zip(*ax.get_legend_handles_labels()):
            if l not in labels:
                handles.append(h)
                labels.append(l)
    order = [l for _, l, _ in ZM if l in labels]
    handles = [handles[labels.index(l)] for l in order]
    labels = order
    fig.legend(handles, labels, loc="lower center", ncol=len(labels),
               frameon=False, fontsize=7.5, labelcolor=INK,
               bbox_to_anchor=(0.5, 0.0))
    fig.suptitle(title, x=0.02, ha="left", fontsize=10.5, color=INK)
    fig.subplots_adjust(left=0.07, right=0.94, top=0.88 if rows > 1 else 0.8,
                        bottom=0.13 if rows > 1 else 0.27,
                        wspace=0.42, hspace=0.6)
    return fig


def end_labels(ax, xs, ends, fmt, log):
    """values at the end of each line, pushed apart so they never overlap:
    by a fixed step on a linear axis, by a fixed factor on a log one"""
    if log:
        ax.set_yscale("log")
        ax.yaxis.set_major_formatter(plt.FuncFormatter(lambda v, _: f"{v:g}"))
    else:
        ax.set_ylim(bottom=0)
    lo, hi = ax.get_ylim()
    gap = (hi / lo) ** 0.07 if log else (hi - lo) * 0.08
    placed = []
    for y in sorted(ends):
        if placed:
            pos = max(y, placed[-1] * gap if log else placed[-1] + gap)
        else:
            pos = y
        placed.append(pos)
        ax.annotate(fmt(y), (xs[-1], y), xytext=(xs[-1] + 0.25, pos),
                    va="center", fontsize=6, color=INK)


def per_malloc(col):
    return lambda r: int(r[col]) / (int(r["ops"]) // 2)


def relative(data, col, traces):
    base = {t: float(data[("glibc", t)][col]) for t in traces}
    return lambda r: float(r[col]) / base[r["trace"]]


def times(v, _=None):
    return f"{v:.2f}×"


def scaling(data):
    panels = (
        ("speed relative to glibc",
         relative(data, "ops_per_sec", SCALE), times, True),
        ("million ops/sec", lambda r: float(r["ops_per_sec"]) / 1e6,
         lambda v: f"{v:.1f}", True),
        ("blocks visited per malloc", per_malloc("blocks_scanned"),
         lambda v: f"{v:.0f}", False),
        ("splits per malloc", per_malloc("splits"),
         lambda v: f"{v:.2f}", False),
        ("resident memory relative to glibc",
         relative(data, "rss_kb", SCALE), times, True),
        ("peak utilization", lambda r: float(r["peak_util"]),
         lambda v: f"{v:.0%}", True),
    )
    fig = lines(data, SCALE, SCALE_X, "blocks live", panels,
                "Random 1 B-4 KiB with an exact live set, 1k to 128k blocks")
    return fig


def hardware(prof):
    ipc = lambda r: float(r["instr_per_op"]) / float(r["cycles_per_op"])  # noqa: E731
    col = lambda c: lambda r: float(r[c])  # noqa: E731
    panels = (
        ("cycles per op (log)", col("cycles_per_op"), lambda v: f"{v:.0f}",
         True, "log"),
        ("instructions per cycle", ipc, lambda v: f"{v:.2f}", True),
        ("L1D load misses per op (log)", col("l1d_miss_per_op"),
         lambda v: f"{v:.1f}", True, "log"),
        ("cache misses per op (log, generic)", col("cache_miss_per_op"),
         lambda v: f"{v:.1f}", True, "log"),
        ("dTLB load misses per op (log)", col("dtlb_miss_per_op"),
         lambda v: f"{v:.1f}", True, "log"),
        ("branch misses per op", col("branch_miss_per_op"),
         lambda v: f"{v:.2f}", True),
    )
    return lines(prof, SCALE, SCALE_X, "blocks live", panels,
                 "CPU counters per operation on the same traces "
                 "(profiling pass, user space)")


def recycle(data):
    """pinned vs freed is the controlled pair: same sizes, same ops apart from
    freeing the separators, so a gap between the two panels is the cost of
    merging and splitting, nothing else"""
    pinned = tuple(f"recycle-{s}-pinned" for s in RECYCLE)
    freed = tuple(f"recycle-{s}-freed" for s in RECYCLE)
    panels = (
        ("speed vs glibc, neighbours pinned",
         relative(data, "ops_per_sec", pinned), times, True),
        ("speed vs glibc, neighbours freed",
         relative(data, "ops_per_sec", freed), times, True),
        ("splits per malloc, neighbours freed", per_malloc("splits"),
         lambda v: f"{v:.2f}", False),
    )
    return lines_multi(
        [(data, pinned), (data, freed), (data, freed)],
        ("32", "64", "128", "256", "512", "1K", "mix"), "request size (B)",
        panels,
        "Exact-size recycling: neighbours pinned (no merging possible) "
        "vs freed")


def lines_multi(sources, xlabels, xlabel, panels, title):
    """like lines(), but every panel has its own trace list"""
    fig, axes = plt.subplots(1, len(panels), figsize=(6.5, 3.0))
    for ax, (src, traces), panel in zip(axes, sources, panels):
        ptitle, value, fmt, with_glibc = panel[:4]
        log = len(panel) > 4 and panel[4] == "log"
        xs = range(len(traces))
        ends = []
        for ver, label, color in ZM:
            if ver == "glibc" and not with_glibc:
                continue
            ys = [value(src[(ver, t)]) for t in traces]
            ax.plot(xs, ys, color=color, linewidth=2, marker="o",
                    markersize=4.5, markeredgecolor="white",
                    markeredgewidth=1, label=label,
                    zorder=3 if ver != "glibc" else 2)
            ends.append(ys[-1])
        end_labels(ax, xs, ends, fmt, log)
        ax.set_xticks(list(xs))
        ax.set_xticklabels(xlabels, fontsize=6, rotation=45)
        ax.set_xlim(-0.3, len(traces) - 0.2)
        ax.set_xlabel(xlabel, fontsize=7, color=MUTED)
        ax.set_title(ptitle, loc="left", fontsize=7.5, color=INK, pad=5)
        clean(ax)
        ax.spines["left"].set_visible(True)
        ax.spines["left"].set_color(GRID)
        ax.tick_params(axis="y", labelsize=6.5, colors=MUTED, length=0)
        ax.tick_params(axis="x", colors=MUTED)
        ax.grid(axis="y", color=GRID, linewidth=0.8)
        ax.grid(axis="x", visible=False)
    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc="lower center", ncol=len(labels),
               frameon=False, fontsize=7.5, labelcolor=INK,
               bbox_to_anchor=(0.5, 0.0))
    fig.suptitle(title, x=0.02, ha="left", fontsize=10, color=INK)
    fig.subplots_adjust(left=0.07, right=0.94, top=0.8, bottom=0.27,
                        wspace=0.42)
    return fig


def speed_fmt(v):
    return f"{v:.1f}" if v >= 1 else f"{v:.2f}"


def main():
    data = load()
    os.makedirs(OUT, exist_ok=True)
    plt.rcParams["font.family"] = "DejaVu Sans"
    figs = {
        "tradeoff": tradeoff(data),
        "speed": per_trace(
            data,
            lambda d, v, t: float(d[(v, t)]["ops_per_sec"]) / 1e6,
            speed_fmt,
            "Throughput per trace, million ops/sec (higher is faster)"),
        "util": per_trace(
            data,
            lambda d, v, t: float(d[(v, t)]["peak_util"]),
            lambda v: f"{v:.0%}",
            "Peak utilization per trace: most bytes live ÷ most heap held "
            "(higher is better)",
            xlim=1.3),
    }
    if all(("glibc", t) in data for t in SCALE):
        figs["scaling"] = scaling(data)
    if all(("glibc", f"recycle-{r}-freed") in data for r in RECYCLE):
        figs["recycle"] = recycle(data)
    if all(("glibc", t) in data for t, _ in DIST):
        figs["dist-speed"] = per_trace(
            data,
            lambda d, v, t: float(d[(v, t)]["ops_per_sec"]) / 1e6,
            speed_fmt,
            "Size distributions: million ops/sec (higher is faster)",
            traces=DIST)
        figs["dist-util"] = per_trace(
            data,
            lambda d, v, t: float(d[(v, t)]["peak_util"]),
            lambda v: f"{v:.0%}",
            "Size distributions: peak utilization (higher is better)",
            xlim=1.3, traces=DIST)
    if os.path.exists("results/profile.csv"):
        with open("results/profile.csv") as f:
            prof = {(r["version"], r["trace"]): r for r in csv.DictReader(f)}
        if all(("glibc", t) in prof for t in SCALE):
            figs["hardware"] = hardware(prof)
    for name, fig in figs.items():
        for ext in ("pdf", "png"):
            fig.savefig(f"{OUT}/{name}.{ext}", dpi=200, facecolor="white")
        plt.close(fig)
        print(f"wrote {OUT}/{name}.pdf and .png")


if __name__ == "__main__":
    main()
