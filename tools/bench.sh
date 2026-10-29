#!/bin/sh
# run every given bench binary over every trace, one process per trace.
# usage: tools/bench.sh bench_glibc [bench_implicit ...]
# env: RUNS (default 10), PIN (e.g. "taskset -c 2"), OUT (csv path),
#      FLAGS (extra driver flags, e.g. -p for the profiling pass),
#      BUDGET (timed seconds after which a trace stops early, 3 runs min)
set -u

[ $# -gt 0 ] || { echo "usage: $0 bench_X..." >&2; exit 2; }

RUNS=${RUNS:-10}
PIN=${PIN:-}
FLAGS=${FLAGS:-}
BUDGET=${BUDGET:-2}
mkdir -p results
OUT=${OUT:-results/bench-$(date +%Y%m%d-%H%M%S).csv}

"./$1" -H > "$OUT"
fail=0
for bin in "$@"; do
	for t in traces/*.trace traces/gen/*.trace; do
		[ -f "$t" ] || continue
		# implicit walks every block per malloc: 1490 blocks per malloc at 4k
		# live already, minutes per trace from 8k up, and it is known O(n)
		case "$bin:$(basename "$t")" in
		bench_implicit:scale-8k.trace | bench_implicit:scale-16k.trace | \
			bench_implicit:scale-32k.trace | bench_implicit:scale-64k.trace | \
			bench_implicit:scale-128k.trace) continue ;;
		esac
		if ! $PIN "./$bin" $FLAGS -n "$RUNS" -T "$BUDGET" "$t" >> "$OUT"; then
			echo "bench: $bin failed on $t" >&2
			fail=1
		fi
	done
done

echo "wrote $OUT" >&2
exit $fail
