#!/usr/bin/env python3
"""generate the synthetic benchmark traces.

usage: gentrace.py [outdir]   (default: traces/gen)

every trace is seeded, so the same files come out on every machine. every
trace frees everything it allocates.

two suites:
  main    small-fixed .. throughput. sizes stay under 128 KiB so glibc never
          switches to its own mmap path: same footing for both, differences
          come from the free-list design (see MIDTERM_PLAN.md, Q1).
  stress  scale-*, holes-*, large-holes-*, huge-churn. no handicaps, built
          to find where tmalloc falls apart against glibc and why.
"""

import os
import random
import sys


class Trace:
    def __init__(self, seed):
        self.rng = random.Random(seed)
        self.ops = []
        self.live = []  # ids currently allocated
        self.next_id = 0

    def alloc(self, size):
        i = self.next_id
        self.next_id += 1
        self.ops.append(f"a {i} {size}")
        self.live.append(i)
        return i

    def free(self, i):
        self.ops.append(f"f {i}")
        self.live.remove(i)

    def free_random(self):
        # swap-remove keeps this O(1) for the big traces
        k = self.rng.randrange(len(self.live))
        self.live[k], self.live[-1] = self.live[-1], self.live[k]
        i = self.live.pop()
        self.ops.append(f"f {i}")

    def free_all(self):
        while self.live:
            self.free_random()

    def steady(self, n_ops, target_live, size_fn):
        """alloc until target_live blocks are live, then alloc/free at random"""
        while len(self.ops) < n_ops:
            if len(self.live) < target_live // 2:
                grow = True
            elif len(self.live) >= target_live:
                grow = False
            else:
                grow = self.rng.random() < 0.5
            if grow:
                self.alloc(size_fn())
            else:
                self.free_random()
        self.free_all()

    def write(self, path, comment):
        with open(path, "w") as f:
            f.write(f"# {comment}\n")
            f.write("\n".join(self.ops))
            f.write("\n")


def churn_trace(seed, live, pairs, size_fn, separators=None):
    """exact live set: allocate `live` blocks, then `pairs` times free a
    random one and allocate a replacement, then free everything. the live
    count never moves during the churn, so every op after the build-up is
    steady state. own bookkeeping, so the generators below stay identical.

    separators: None, "pinned" (a 32 B block after every slot that stays
    allocated, so a freed slot can't merge and the next request of the same
    size reuses it whole) or "freed" (the same separators, freed before the
    churn, so freed slots merge with them and get split again)."""
    t = Trace(seed)
    slots, seps = [], []
    for _ in range(live):
        slots.append(t.alloc(size_fn()))
        if separators:
            seps.append(t.alloc(32))
    if separators == "freed":
        for s in seps:
            t.ops.append(f"f {s}")
        seps = []
    assert len(slots) == live
    for _ in range(pairs):
        k = t.rng.randrange(live)
        t.ops.append(f"f {slots[k]}")
        slots[k] = t.alloc(size_fn())
    rest = slots + seps
    t.rng.shuffle(rest)
    for i in rest:
        t.ops.append(f"f {i}")
    t.live = []
    return t


def small_fixed():
    t = Trace(1)
    t.steady(100_000, 1000, lambda: t.rng.choice((16, 32, 48, 64)))
    return t, "small fixed sizes (16-64 B), steady ~1000 live, the easy case"


def random_sizes():
    t = Trace(2)
    t.steady(100_000, 2000, lambda: t.rng.randint(1, 4096))
    return t, "uniform random sizes 1 B - 4 KiB, random free order"


def bimodal():
    t = Trace(3)

    def size():
        if t.rng.random() < 0.9:
            return t.rng.randint(8, 64)
        return t.rng.randint(4096, 64 * 1024)

    t.steady(100_000, 2000, size)
    return t, "bimodal: 90% tiny (8-64 B), 10% large (4-64 KiB)"


def grow_free_all():
    t = Trace(4)
    for _ in range(4):
        for _ in range(5000):
            t.alloc(t.rng.randint(16, 2048))
        t.free_all()
    return t, "4 rounds of: alloc 5000 blocks, free all in random order"


def checkerboard():
    t = Trace(5)
    for _ in range(2):
        small, large = [], []
        for _ in range(5000):
            small.append(t.alloc(32))
            large.append(t.alloc(512))
        for i in small:
            t.free(i)
        # 256 B does not fit in a 32 B hole, so the heap has to grow
        for _ in range(5000):
            t.alloc(256)
        t.free_all()
    return t, "alternate 32 B / 512 B, free the 32s, then alloc 256 B"


def throughput():
    t = Trace(6)
    t.steady(1_000_000, 1000, lambda: t.rng.randint(16, 1024))
    return t, "1M ops, sizes 16 B - 1 KiB, steady ~1000 live"


def scale(live):
    """random 1 B - 4 KiB with an exact live set: how the lists scale. 250k
    churn pairs at every size, so even 128k has 2x its live set in steady
    state ops (the checkpoint's version drifted toward its target instead)"""
    def gen():
        t = Trace(10 + live)
        t = churn_trace(10 + live, live, 250_000,
                        lambda: t.rng.randint(1, 4096))
        return t, f"random 1 B - 4 KiB, exactly {live} live, 250k churn pairs"
    return gen


RECYCLE_SIZES = (32, 64, 128, 256, 512, 1024)


def recycle(size, separators):
    """exact-size recycling. pinned vs freed is the controlled pair: same
    sizes, same op sequence apart from freeing the separators, so the only
    difference is whether a freed block can merge and has to be split"""
    def gen():
        t = Trace(0)
        if size == "mixed":
            fn = lambda: t.rng.choice(RECYCLE_SIZES)  # noqa: E731
        else:
            fn = lambda: size  # noqa: E731
        t = churn_trace(30 + (0 if size == "mixed" else size), 1000, 200_000,
                        fn, separators)
        return t, f"{size} B recycling, 1000 live, separators {separators}"
    return gen


def frag_grow():
    """20 phases, each 2000 blocks 32 B bigger than the last, every other one
    freed: the holes left behind are all too small for every later request
    and can't merge (their neighbours stay allocated)"""
    t = Trace(21)
    keep = []
    for p in range(20):
        size = 32 + 32 * p
        ids = [t.alloc(size) for _ in range(2000)]
        for i in ids[::2]:
            t.ops.append(f"f {i}")
        keep += ids[1::2]
    t.rng.shuffle(keep)
    for i in keep:
        t.ops.append(f"f {i}")
    t.live = []
    return t, "20 growing-size phases, half of each freed as unusable holes"


def dist(name):
    """the same steady churn (2000 live, 200k pairs) under different size
    distributions, to see whether uniform random favours someone"""
    def size_fn(rng):
        if name == "small":
            if rng.random() < 0.9:
                return rng.randint(8, 64)
            return rng.randint(65, 256)
        if name == "bimodal":
            if rng.random() < 0.7:
                return rng.randint(16, 128)
            return rng.randint(1024, 4096)
        if name == "loguniform":
            return int(2 ** rng.uniform(4, 14))
        if name == "classes":
            return rng.choice((16, 32, 48, 64, 96, 128, 192, 256, 384, 512,
                               768, 1024))
        # boundary: block sizes at the bottom (2^k) and the top (2^(k+1)-16)
        # of segregated's power-of-two classes, the worst case for its
        # first fit inside a class
        k = rng.randint(6, 12)
        return (1 << k) - 16 if rng.random() < 0.5 else (1 << (k + 1)) - 32

    def gen():
        t = Trace(0)
        t = churn_trace(40 + len(name), 2000, 200_000,
                        lambda: size_fn(t.rng))
        return t, f"size distribution: {name}, 2000 live, 200k churn pairs"
    return gen


def holes(n):
    """lifo worst case: n unmergeable 32 B holes at the front of the free
    list, then large requests that none of them can serve"""
    def gen():
        t = Trace(99)
        small = []
        for _ in range(n):
            small.append(t.alloc(32))
            t.alloc(32)  # separator, keeps the holes from merging
        for i in small:
            t.free(i)
        for _ in range(2000):
            t.alloc(1024)
        t.free_all()
        return t, f"{n} unmergeable 32 B holes, then 2000 x 1 KiB"
    return gen


def large_holes(n):
    """n unmergeable 40 KiB holes, all on the >= 32 KiB list, then 100 KiB
    requests none of them can serve. 100 KiB is above our 64 KiB chunk, so
    the heap grows by exactly the request and leaves no leftover at the
    front: every malloc walks the whole list. still below 128 KiB, so glibc
    answers from its sorted large bins, not mmap"""
    def gen():
        t = Trace(98)
        big = []
        for _ in range(n):
            big.append(t.alloc(40 * 1024))
            t.alloc(32)  # separator
        for i in big:
            t.free(i)
        for _ in range(500):
            t.alloc(100 * 1024)
        t.free_all()
        return t, f"{n} unmergeable 40 KiB holes, then 500 x 100 KiB"
    return gen


def huge_churn():
    """no 128 KiB cap: glibc mmaps these and gets them back on free, we
    have to fit them all into one heap that never shrinks"""
    t = Trace(7)
    t.steady(20_000, 64, lambda: t.rng.randint(128 * 1024, 4 * 1024 * 1024))
    return t, "128 KiB - 4 MiB, steady ~64 live, glibc uses mmap here"


TRACES = {
    "small-fixed": small_fixed,
    "random": random_sizes,
    "bimodal": bimodal,
    "grow-free-all": grow_free_all,
    "checkerboard": checkerboard,
    "throughput": throughput,
}
# stress traces: where the free lists stop keeping up with glibc
TRACES.update({f"scale-{k}k": scale(k * 1000)
               for k in (1, 2, 4, 8, 16, 32, 64, 128)})
TRACES.update({f"holes-{k}k": holes(k * 1000) for k in (1, 4, 16)})
TRACES.update({f"large-holes-{n}": large_holes(n) for n in (500, 2000, 8000)})
TRACES["huge-churn"] = huge_churn
# extended experiments
for s in RECYCLE_SIZES + ("mixed",):
    for sep in ("pinned", "freed"):
        TRACES[f"recycle-{s}-{sep}"] = recycle(s, sep)
TRACES["frag-grow"] = frag_grow
for d in ("small", "bimodal", "loguniform", "classes", "boundary"):
    TRACES[f"dist-{d}"] = dist(d)


def live_profile(ops):
    """peak live blocks and how many ops run at that peak, read back from
    the trace itself so a generator can't silently miss its target"""
    live = peak = at_peak = 0
    for op in ops:
        live += 1 if op[0] == "a" else -1
        if live > peak:
            peak, at_peak = live, 0
        elif live == peak or live == peak - 1:
            at_peak += 1
    return peak, at_peak


def main():
    outdir = sys.argv[1] if len(sys.argv) > 1 else "traces/gen"
    os.makedirs(outdir, exist_ok=True)
    for name, gen in TRACES.items():
        t, comment = gen()
        path = os.path.join(outdir, f"{name}.trace")
        t.write(path, comment)
        peak, steady = live_profile(t.ops)
        print(f"{path}: {len(t.ops)} ops, peak {peak} live, "
              f"{steady} ops at the peak")


if __name__ == "__main__":
    main()
