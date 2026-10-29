#!/bin/sh
# set up a working tree: restore tracked files that were deleted from the
# setup dirs, create output dirs, generate traces, build and test.
# usage: ./setup.sh
set -eu
cd "$(dirname "$0")"

# only these are restored, so a deliberate deletion elsewhere (say a file
# removed in src/ during a refactor) is left alone
DIRS="bench results tests tools traces"

if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
	# shellcheck disable=SC2086
	missing=$(git ls-files --deleted -- $DIRS)
	if [ -n "$missing" ]; then
		echo "setup: restoring from git:"
		echo "$missing" | sed 's/^/  /'
		# shellcheck disable=SC2086
		git ls-files -z --deleted -- $DIRS | xargs -0 -r git checkout --
	fi
else
	echo "setup: not a git repo, cannot restore deleted files" >&2
fi

for f in Makefile src/tmalloc.h src/memlib.c bench/driver.c \
	tools/gentrace.py tools/bench.sh; do
	if [ ! -f "$f" ]; then
		echo "setup: $f is missing and could not be restored" >&2
		exit 1
	fi
done

mkdir -p results traces/gen
make traces
make test
make

echo "setup: done. run 'make bench' to benchmark every built version"
