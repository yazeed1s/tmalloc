#!/bin/sh
# phase 4 final runs on a quiet machine (see MIDTERM_PLAN.md, phase 4):
# 3 throughput rounds, a profiling pass and the independent rss check.
# stops auto-cpufreq (with turbo = auto it flips boost every ~20s, mid-run),
# sets performance with no boost, runs the full bench 3 times, checks the
# runs agree, and starts auto-cpufreq again however the script exits. the
# daemon reapplies /etc/auto-cpufreq.conf on start.
#
# usage: ./quiet-bench.sh        (as yourself, not root, it asks for sudo)
set -eu
cd "$(dirname "$0")"

GOV=/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
BOOST=/sys/devices/system/cpu/cpufreq/boost
ROUNDS=3

if [ "$(id -u)" -eq 0 ]; then
	echo "quiet-bench: run as yourself, not root, or results/ ends up root-owned" >&2
	exit 1
fi

for ac in /sys/class/power_supply/A*/online; do
	if [ -f "$ac" ] && [ "$(cat "$ac")" != 1 ]; then
		echo "quiet-bench: plug in the charger first" >&2
		exit 1
	fi
done

busy=$(pgrep -l -x 'codium|firefox|chromium|code' | awk '{print $2}' | sort -u || true)
if [ -n "$busy" ]; then
	echo "quiet-bench: close these first:" >&2
	echo "$busy" | sed 's/^/  /' >&2
	exit 1
fi

restore() {
	echo "quiet-bench: starting auto-cpufreq again"
	sudo systemctl start auto-cpufreq || true
	sleep 2
	echo "quiet-bench: governor now $(cat $GOV), boost $(cat $BOOST)"
}

sudo -v
# the run takes longer than sudo remembers a password, and the restore at
# the end needs it: refresh it in the background until the script exits
( while sleep 60; do sudo -n -v 2>/dev/null || exit; done ) &
keepalive=$!
trap 'kill $keepalive 2>/dev/null; restore' EXIT
trap 'exit 130' INT TERM

sudo systemctl stop auto-cpufreq
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor >/dev/null
echo 0 | sudo tee $BOOST >/dev/null

# every cpu, not just cpu0, and twice to make sure nothing flips it back
for check in 1 2; do
	for g in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
		if [ "$(cat "$g")" != performance ]; then
			echo "quiet-bench: $g is $(cat "$g")" >&2
			exit 1
		fi
	done
	if [ "$(cat $BOOST)" != 0 ]; then
		echo "quiet-bench: boost is still on" >&2
		exit 1
	fi
	[ $check -eq 1 ] && sleep 5
done
echo "quiet-bench: auto-cpufreq stopped, governor performance, boost off"

i=1
while [ $i -le $ROUNDS ]; do
	echo "quiet-bench: round $i of $ROUNDS"
	PIN="taskset -c 2" OUT=results/midterm-$i.csv make -s bench
	i=$((i + 1))
done

# counters in their own pass, so the throughput rounds above carry none
echo "quiet-bench: profiling pass"
PIN="taskset -c 2" make -s profile

echo "quiet-bench: independent rss check"
make -s rsscheck

# runs agree if every version/trace is within 10% across rounds
python3 - results/midterm-*.csv <<'EOF'
import csv, sys
runs = [{(r["version"], r["trace"]): float(r["ops_per_sec"])
         for r in csv.DictReader(open(p))} for p in sys.argv[1:]]
bad = []
for key in runs[0]:
    if key[1].startswith("tiny") or any(key not in r for r in runs):
        continue  # a few ops, timing is noise by design
    v = [r[key] for r in runs]
    spread = max(v) / min(v)
    if spread > 1.10:
        bad.append((spread, key))
if bad:
    print("quiet-bench: these differ by more than 10% between rounds:")
    for spread, (ver, tr) in sorted(bad, reverse=True):
        print(f"  {ver:16} {tr:14} {spread:.2f}x")
else:
    print("quiet-bench: all rounds agree within 10%")
EOF
