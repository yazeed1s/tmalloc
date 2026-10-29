# checkpoint results

Snapshot of the quiet-machine results used in the first version of the midterm
report (commit 23e3549), before the extended experiments. Kept unchanged so
the extended runs can be compared against them.

- `midterm-{1,2,3}.csv`: the three raw rounds
- `midterm.csv`, `midterm.md`: the merged medians and table
- `figures/`: the figures as they were

Note: the `scale-*` traces in these files are the old definition (random
walk toward the target live set, 300k ops). The extended suite regenerates
them with an exact live set, so the numbers are not directly comparable.
