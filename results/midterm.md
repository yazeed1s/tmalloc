machine: AMD Ryzen 7 PRO 6850U with Radeon Graphics, linux 6.18.57-1-lts, ldd (GNU libc) 2.44, gcc (GCC) 16.2.1 20260810, -O2, pinned with taskset, performance governor, boost off

median of 3 rounds, 10 timed runs each

## main suite

| trace | version | Mops/s | vs glibc | scans/malloc | splits/malloc | peak util | rss MB |
|---|---|---|---|---|---|---|---|
| bimodal | glibc | 46.30 | 1.000x | - | - | 0.867 | 6.3 |
| bimodal | implicit | 1.26 | 0.027x | 611.1 | 0.27 | 0.866 | 6.3 |
| bimodal | explicit | 35.43 | 0.765x | 3.7 | 0.90 | 0.472 | 11.7 |
| bimodal | segregated | 39.32 | 0.849x | 3.6 | 0.25 | 0.781 | 7.1 |
| bimodal | segregated-skip | 33.81 | 0.730x | 1.1 | 0.90 | 0.724 | 7.6 |
| checkerboard | glibc | 37.26 | 1.000x | - | - | 0.884 | 4.0 |
| checkerboard | implicit | 0.03 | 0.001x | 7500.5 | 1.00 | 0.901 | 4.0 |
| checkerboard | explicit | 54.60 | 1.465x | 4.8 | 1.00 | 0.901 | 4.0 |
| checkerboard | segregated | 42.92 | 1.152x | 1.0 | 1.00 | 0.901 | 4.0 |
| checkerboard | segregated-skip | 42.48 | 1.140x | 1.0 | 1.00 | 0.901 | 4.0 |
| grow-free-all | glibc | 21.83 | 1.000x | - | - | 0.964 | 5.0 |
| grow-free-all | implicit | 0.13 | 0.006x | 2500.5 | 1.00 | 0.969 | 5.1 |
| grow-free-all | explicit | 69.72 | 3.193x | 1.0 | 1.00 | 0.969 | 5.1 |
| grow-free-all | segregated | 42.94 | 1.967x | 1.0 | 1.00 | 0.969 | 5.1 |
| grow-free-all | segregated-skip | 41.29 | 1.891x | 1.0 | 1.00 | 0.969 | 5.1 |
| random | glibc | 19.09 | 1.000x | - | - | 0.854 | 3.0 |
| random | implicit | 0.42 | 0.022x | 815.1 | 0.89 | 0.900 | 2.9 |
| random | explicit | 31.66 | 1.659x | 4.3 | 0.99 | 0.783 | 3.4 |
| random | segregated | 15.04 | 0.788x | 14.7 | 0.87 | 0.881 | 3.0 |
| random | segregated-skip | 30.31 | 1.588x | 1.0 | 1.00 | 0.693 | 3.8 |
| small-fixed | glibc | 82.29 | 1.000x | - | - | 0.253 | 0.0 |
| small-fixed | implicit | 2.32 | 0.028x | 344.0 | 0.14 | 0.522 | 0.1 |
| small-fixed | explicit | 53.27 | 0.647x | 1.9 | 0.60 | 0.522 | 0.1 |
| small-fixed | segregated | 50.87 | 0.618x | 2.2 | 0.13 | 0.522 | 0.1 |
| small-fixed | segregated-skip | 37.60 | 0.457x | 1.0 | 0.90 | 0.261 | 0.1 |
| throughput | glibc | 88.63 | 1.000x | - | - | 0.575 | 0.8 |
| throughput | implicit | 0.80 | 0.009x | 523.9 | 0.71 | 0.830 | 0.6 |
| throughput | explicit | 39.54 | 0.446x | 2.8 | 0.96 | 0.755 | 0.7 |
| throughput | segregated | 21.96 | 0.248x | 10.5 | 0.71 | 0.830 | 0.6 |
| throughput | segregated-skip | 33.14 | 0.374x | 1.0 | 0.99 | 0.639 | 0.8 |

rankings (fastest first):

- bimodal: glibc > segregated > explicit > segregated-skip > implicit
- checkerboard: explicit > segregated > segregated-skip > glibc > implicit
- grow-free-all: explicit > segregated > segregated-skip > glibc > implicit
- random: explicit > segregated-skip > glibc > segregated > implicit
- small-fixed: glibc > explicit > segregated > segregated-skip > implicit
- throughput: glibc > explicit > segregated-skip > segregated > implicit

## scaling (exact live set)

| trace | version | Mops/s | vs glibc | scans/malloc | splits/malloc | peak util | rss MB |
|---|---|---|---|---|---|---|---|
| scale-128k | glibc | 7.17 | 1.000x | - | - | 0.980 | 255.6 |
| scale-128k | explicit | 0.41 | 0.057x | 125.0 | 0.99 | 0.745 | 336.5 |
| scale-128k | segregated | 0.08 | 0.010x | 723.4 | 0.91 | 0.867 | 289.1 |
| scale-128k | segregated-skip | 7.17 | 1.001x | 1.0 | 1.00 | 0.701 | 357.7 |
| scale-16k | glibc | 13.91 | 1.000x | - | - | 0.956 | 33.1 |
| scale-16k | explicit | 7.94 | 0.571x | 12.1 | 0.99 | 0.742 | 42.8 |
| scale-16k | segregated | 0.99 | 0.071x | 166.3 | 0.87 | 0.865 | 36.8 |
| scale-16k | segregated-skip | 9.83 | 0.706x | 1.0 | 1.00 | 0.698 | 45.5 |
| scale-1k | glibc | 22.02 | 1.000x | - | - | 0.781 | 2.5 |
| scale-1k | implicit | 0.49 | 0.022x | 709.4 | 0.89 | 0.871 | 2.3 |
| scale-1k | explicit | 45.78 | 2.079x | 2.6 | 0.99 | 0.719 | 2.9 |
| scale-1k | segregated | 15.91 | 0.723x | 14.8 | 0.87 | 0.848 | 2.4 |
| scale-1k | segregated-skip | 25.45 | 1.156x | 1.0 | 1.00 | 0.675 | 3.1 |
| scale-2k | glibc | 20.17 | 1.000x | - | - | 0.853 | 4.7 |
| scale-2k | implicit | 0.24 | 0.012x | 1401.0 | 0.87 | 0.891 | 4.6 |
| scale-2k | explicit | 35.79 | 1.774x | 3.9 | 0.99 | 0.731 | 5.6 |
| scale-2k | segregated | 10.03 | 0.497x | 26.0 | 0.87 | 0.856 | 4.8 |
| scale-2k | segregated-skip | 24.78 | 1.228x | 1.0 | 1.00 | 0.692 | 5.9 |
| scale-32k | glibc | 12.70 | 1.000x | - | - | 0.970 | 64.9 |
| scale-32k | explicit | 3.42 | 0.269x | 21.9 | 0.99 | 0.742 | 85.1 |
| scale-32k | segregated | 0.37 | 0.029x | 297.9 | 0.88 | 0.866 | 72.8 |
| scale-32k | segregated-skip | 8.69 | 0.684x | 1.0 | 1.00 | 0.700 | 90.2 |
| scale-4k | glibc | 17.87 | 1.000x | - | - | 0.903 | 8.8 |
| scale-4k | implicit | 0.11 | 0.006x | 2771.5 | 0.85 | 0.899 | 8.9 |
| scale-4k | explicit | 25.01 | 1.399x | 5.1 | 0.99 | 0.739 | 10.9 |
| scale-4k | segregated | 5.26 | 0.294x | 47.5 | 0.87 | 0.857 | 9.4 |
| scale-4k | segregated-skip | 17.04 | 0.953x | 1.0 | 1.00 | 0.691 | 11.6 |
| scale-64k | glibc | 10.41 | 1.000x | - | - | 0.972 | 128.8 |
| scale-64k | explicit | 1.21 | 0.116x | 48.1 | 0.99 | 0.738 | 169.9 |
| scale-64k | segregated | 0.14 | 0.013x | 498.1 | 0.89 | 0.864 | 145.1 |
| scale-64k | segregated-skip | 8.44 | 0.811x | 1.0 | 1.00 | 0.693 | 180.7 |
| scale-8k | glibc | 15.11 | 1.000x | - | - | 0.929 | 17.0 |
| scale-8k | explicit | 14.26 | 0.944x | 7.9 | 0.99 | 0.738 | 21.6 |
| scale-8k | segregated | 2.22 | 0.147x | 90.5 | 0.87 | 0.860 | 18.4 |
| scale-8k | segregated-skip | 11.87 | 0.786x | 1.0 | 1.00 | 0.694 | 22.9 |

rankings (fastest first):

- scale-128k: segregated-skip > glibc > explicit > segregated
- scale-16k: glibc > segregated-skip > explicit > segregated
- scale-1k: explicit > segregated-skip > glibc > segregated > implicit
- scale-2k: explicit > segregated-skip > glibc > segregated > implicit
- scale-32k: glibc > segregated-skip > explicit > segregated
- scale-4k: explicit > glibc > segregated-skip > segregated > implicit
- scale-64k: glibc > segregated-skip > explicit > segregated
- scale-8k: glibc > explicit > segregated-skip > segregated

## holes, large holes, huge blocks

| trace | version | Mops/s | vs glibc | scans/malloc | splits/malloc | peak util | rss MB |
|---|---|---|---|---|---|---|---|
| holes-16k | glibc | 40.46 | 1.000x | - | - | 0.702 | 3.4 |
| holes-16k | implicit | 0.05 | 0.001x | 17000.5 | 1.00 | 0.698 | 3.4 |
| holes-16k | explicit | 17.26 | 0.427x | 16.5 | 1.00 | 0.698 | 3.4 |
| holes-16k | segregated | 44.41 | 1.098x | 1.0 | 1.00 | 0.698 | 3.4 |
| holes-16k | segregated-skip | 47.33 | 1.170x | 1.0 | 1.00 | 0.698 | 3.4 |
| holes-1k | glibc | 38.18 | 1.000x | - | - | 0.905 | 2.1 |
| holes-1k | implicit | 0.43 | 0.011x | 2000.5 | 1.00 | 0.933 | 2.1 |
| holes-1k | explicit | 56.09 | 1.469x | 9.2 | 1.00 | 0.933 | 2.1 |
| holes-1k | segregated | 65.58 | 1.717x | 1.0 | 1.00 | 0.933 | 2.1 |
| holes-1k | segregated-skip | 64.69 | 1.694x | 1.0 | 1.00 | 0.933 | 2.1 |
| holes-4k | glibc | 41.26 | 1.000x | - | - | 0.847 | 2.3 |
| holes-4k | implicit | 0.17 | 0.004x | 5000.5 | 1.00 | 0.874 | 2.4 |
| holes-4k | explicit | 32.41 | 0.786x | 14.2 | 1.00 | 0.874 | 2.4 |
| holes-4k | segregated | 51.35 | 1.245x | 1.0 | 1.00 | 0.874 | 2.4 |
| holes-4k | segregated-skip | 54.29 | 1.316x | 1.0 | 1.00 | 0.874 | 2.4 |
| huge-churn | glibc | 0.56 | 1.000x | - | - | 0.849 | 179.4 |
| huge-churn | implicit | 10.45 | 18.574x | 38.3 | 0.99 | 0.856 | 178.0 |
| huge-churn | explicit | 44.17 | 78.484x | 1.9 | 0.99 | 0.768 | 198.0 |
| huge-churn | segregated | 28.95 | 51.444x | 1.9 | 0.99 | 0.768 | 198.0 |
| huge-churn | segregated-skip | 29.01 | 51.545x | 1.9 | 0.99 | 0.768 | 198.0 |
| large-holes-2000 | glibc | 6.78 | 1.000x | - | - | 0.631 | 123.7 |
| large-holes-2000 | implicit | 0.11 | 0.016x | 2250.5 | 1.00 | 0.615 | 127.1 |
| large-holes-2000 | explicit | 2.20 | 0.325x | 223.2 | 1.00 | 0.615 | 127.1 |
| large-holes-2000 | segregated | 3.07 | 0.452x | 222.9 | 1.00 | 0.615 | 127.1 |
| large-holes-2000 | segregated-skip | 2.87 | 0.424x | 222.9 | 1.00 | 0.615 | 127.1 |
| large-holes-500 | glibc | 2.07 | 1.000x | - | - | 0.749 | 65.1 |
| large-holes-500 | implicit | 0.46 | 0.224x | 750.5 | 1.00 | 0.714 | 68.4 |
| large-holes-500 | explicit | 5.63 | 2.723x | 167.7 | 1.00 | 0.714 | 68.4 |
| large-holes-500 | segregated | 5.24 | 2.535x | 167.2 | 1.00 | 0.714 | 68.4 |
| large-holes-500 | segregated-skip | 5.62 | 2.716x | 167.2 | 1.00 | 0.714 | 68.4 |
| large-holes-8000 | glibc | 6.67 | 1.000x | - | - | 0.872 | 358.5 |
| large-holes-8000 | implicit | 0.01 | 0.002x | 8250.5 | 1.00 | 0.864 | 361.8 |
| large-holes-8000 | explicit | 1.31 | 0.196x | 243.4 | 1.00 | 0.864 | 361.8 |
| large-holes-8000 | segregated | 1.31 | 0.197x | 243.2 | 1.00 | 0.864 | 361.8 |
| large-holes-8000 | segregated-skip | 1.31 | 0.197x | 243.2 | 1.00 | 0.864 | 361.8 |

rankings (fastest first):

- holes-16k: segregated-skip > segregated > glibc > explicit > implicit
- holes-1k: segregated > segregated-skip > explicit > glibc > implicit
- holes-4k: segregated-skip > segregated > glibc > explicit > implicit
- huge-churn: explicit > segregated-skip > segregated > implicit > glibc
- large-holes-2000: glibc > segregated > segregated-skip > explicit > implicit
- large-holes-500: explicit > segregated-skip > segregated > glibc > implicit
- large-holes-8000: glibc > segregated-skip > segregated > explicit > implicit

## exact-size recycling (pinned vs freed separators)

| trace | version | Mops/s | vs glibc | scans/malloc | splits/malloc | peak util | rss MB |
|---|---|---|---|---|---|---|---|
| recycle-1024-freed | glibc | 141.11 | 1.000x | - | - | 0.868 | 1.0 |
| recycle-1024-freed | implicit | 1.08 | 0.008x | 537.4 | 0.15 | 0.948 | 1.0 |
| recycle-1024-freed | explicit | 126.26 | 0.895x | 1.0 | 0.15 | 0.948 | 1.0 |
| recycle-1024-freed | segregated | 81.04 | 0.574x | 1.0 | 0.15 | 0.948 | 1.0 |
| recycle-1024-freed | segregated-skip | 36.41 | 0.258x | 1.0 | 1.00 | 0.644 | 1.6 |
| recycle-1024-pinned | glibc | 140.37 | 1.000x | - | - | 0.868 | 1.0 |
| recycle-1024-pinned | implicit | 0.37 | 0.003x | 998.7 | 0.01 | 0.948 | 1.0 |
| recycle-1024-pinned | explicit | 151.02 | 1.076x | 1.0 | 0.01 | 0.948 | 1.0 |
| recycle-1024-pinned | segregated | 101.44 | 0.723x | 1.0 | 0.01 | 0.948 | 1.0 |
| recycle-1024-pinned | segregated-skip | 40.32 | 0.287x | 1.0 | 1.00 | 0.393 | 2.6 |
| recycle-128-freed | glibc | 169.30 | 1.000x | - | - | 0.592 | 0.2 |
| recycle-128-freed | implicit | 1.13 | 0.007x | 524.8 | 0.10 | 0.814 | 0.2 |
| recycle-128-freed | explicit | 140.65 | 0.831x | 1.0 | 0.15 | 0.814 | 0.2 |
| recycle-128-freed | segregated | 95.48 | 0.564x | 1.0 | 0.09 | 0.814 | 0.2 |
| recycle-128-freed | segregated-skip | 35.12 | 0.207x | 1.0 | 1.00 | 0.610 | 0.2 |
| recycle-128-pinned | glibc | 166.91 | 1.000x | - | - | 0.592 | 0.2 |
| recycle-128-pinned | implicit | 0.56 | 0.003x | 999.2 | 0.01 | 0.814 | 0.2 |
| recycle-128-pinned | explicit | 168.20 | 1.008x | 1.0 | 0.01 | 0.814 | 0.2 |
| recycle-128-pinned | segregated | 117.16 | 0.702x | 1.0 | 0.01 | 0.814 | 0.2 |
| recycle-128-pinned | segregated-skip | 44.23 | 0.265x | 1.0 | 1.00 | 0.349 | 0.4 |
| recycle-256-freed | glibc | 159.09 | 1.000x | - | - | 0.710 | 0.3 |
| recycle-256-freed | implicit | 1.16 | 0.007x | 534.4 | 0.13 | 0.879 | 0.3 |
| recycle-256-freed | explicit | 135.33 | 0.851x | 1.0 | 0.15 | 0.879 | 0.3 |
| recycle-256-freed | segregated | 83.03 | 0.522x | 1.1 | 0.13 | 0.879 | 0.3 |
| recycle-256-freed | segregated-skip | 34.31 | 0.216x | 1.0 | 1.00 | 0.628 | 0.4 |
| recycle-256-pinned | glibc | 158.21 | 1.000x | - | - | 0.710 | 0.3 |
| recycle-256-pinned | implicit | 0.39 | 0.002x | 1001.5 | 0.01 | 0.879 | 0.3 |
| recycle-256-pinned | explicit | 161.90 | 1.023x | 1.0 | 0.01 | 0.879 | 0.3 |
| recycle-256-pinned | segregated | 107.57 | 0.680x | 1.0 | 0.01 | 0.879 | 0.3 |
| recycle-256-pinned | segregated-skip | 34.57 | 0.219x | 1.0 | 1.00 | 0.366 | 0.7 |
| recycle-32-freed | glibc | 170.17 | 1.000x | - | - | 0.473 | 0.1 |
| recycle-32-freed | implicit | 1.68 | 0.010x | 505.7 | 0.01 | 0.488 | 0.1 |
| recycle-32-freed | explicit | 146.03 | 0.858x | 1.0 | 0.15 | 0.488 | 0.1 |
| recycle-32-freed | segregated | 115.12 | 0.677x | 1.0 | 0.02 | 0.488 | 0.1 |
| recycle-32-freed | segregated-skip | 45.49 | 0.267x | 1.0 | 1.00 | 0.488 | 0.1 |
| recycle-32-pinned | glibc | 167.69 | 1.000x | - | - | 0.473 | 0.1 |
| recycle-32-pinned | implicit | 0.86 | 0.005x | 1001.9 | 0.01 | 0.488 | 0.1 |
| recycle-32-pinned | explicit | 176.03 | 1.050x | 1.0 | 0.01 | 0.488 | 0.1 |
| recycle-32-pinned | segregated | 116.47 | 0.695x | 1.0 | 0.01 | 0.488 | 0.1 |
| recycle-32-pinned | segregated-skip | 46.41 | 0.277x | 1.0 | 1.00 | 0.326 | 0.2 |
| recycle-512-freed | glibc | 150.43 | 1.000x | - | - | 0.805 | 0.5 |
| recycle-512-freed | implicit | 1.11 | 0.007x | 537.4 | 0.14 | 0.922 | 0.6 |
| recycle-512-freed | explicit | 128.69 | 0.855x | 1.0 | 0.15 | 0.922 | 0.6 |
| recycle-512-freed | segregated | 84.23 | 0.560x | 1.0 | 0.14 | 0.922 | 0.6 |
| recycle-512-freed | segregated-skip | 36.96 | 0.246x | 1.0 | 1.00 | 0.638 | 0.8 |
| recycle-512-pinned | glibc | 146.99 | 1.000x | - | - | 0.805 | 0.5 |
| recycle-512-pinned | implicit | 0.38 | 0.003x | 1001.5 | 0.01 | 0.922 | 0.6 |
| recycle-512-pinned | explicit | 154.73 | 1.053x | 1.0 | 0.01 | 0.922 | 0.6 |
| recycle-512-pinned | segregated | 101.22 | 0.689x | 1.0 | 0.01 | 0.922 | 0.6 |
| recycle-512-pinned | segregated-skip | 33.57 | 0.228x | 1.0 | 1.00 | 0.377 | 1.3 |
| recycle-64-freed | glibc | 168.67 | 1.000x | - | - | 0.710 | 0.1 |
| recycle-64-freed | implicit | 1.60 | 0.009x | 512.0 | 0.04 | 0.732 | 0.1 |
| recycle-64-freed | explicit | 139.39 | 0.826x | 1.0 | 0.15 | 0.732 | 0.1 |
| recycle-64-freed | segregated | 97.00 | 0.575x | 1.4 | 0.05 | 0.732 | 0.1 |
| recycle-64-freed | segregated-skip | 38.39 | 0.228x | 1.0 | 1.00 | 0.732 | 0.1 |
| recycle-64-pinned | glibc | 166.69 | 1.000x | - | - | 0.710 | 0.1 |
| recycle-64-pinned | implicit | 0.85 | 0.005x | 1000.9 | 0.01 | 0.732 | 0.1 |
| recycle-64-pinned | explicit | 172.19 | 1.033x | 1.0 | 0.01 | 0.732 | 0.1 |
| recycle-64-pinned | segregated | 114.31 | 0.686x | 1.0 | 0.01 | 0.732 | 0.1 |
| recycle-64-pinned | segregated-skip | 36.31 | 0.218x | 1.0 | 1.00 | 0.366 | 0.2 |
| recycle-mixed-freed | glibc | 107.84 | 1.000x | - | - | 0.690 | 0.4 |
| recycle-mixed-freed | implicit | 0.93 | 0.009x | 565.8 | 0.32 | 0.814 | 0.4 |
| recycle-mixed-freed | explicit | 52.73 | 0.489x | 2.0 | 0.94 | 0.570 | 0.6 |
| recycle-mixed-freed | segregated | 51.57 | 0.478x | 2.6 | 0.23 | 0.814 | 0.4 |
| recycle-mixed-freed | segregated-skip | 30.47 | 0.282x | 1.0 | 0.93 | 0.518 | 0.7 |
| recycle-mixed-pinned | glibc | 109.20 | 1.000x | - | - | 0.750 | 0.5 |
| recycle-mixed-pinned | implicit | 0.49 | 0.004x | 1043.2 | 0.24 | 0.687 | 0.5 |
| recycle-mixed-pinned | explicit | 51.97 | 0.476x | 1.9 | 0.92 | 0.442 | 0.9 |
| recycle-mixed-pinned | segregated | 69.61 | 0.637x | 1.2 | 0.04 | 0.773 | 0.5 |
| recycle-mixed-pinned | segregated-skip | 36.69 | 0.336x | 1.0 | 0.93 | 0.364 | 1.0 |

rankings (fastest first):

- recycle-1024-freed: glibc > explicit > segregated > segregated-skip > implicit
- recycle-1024-pinned: explicit > glibc > segregated > segregated-skip > implicit
- recycle-128-freed: glibc > explicit > segregated > segregated-skip > implicit
- recycle-128-pinned: explicit > glibc > segregated > segregated-skip > implicit
- recycle-256-freed: glibc > explicit > segregated > segregated-skip > implicit
- recycle-256-pinned: explicit > glibc > segregated > segregated-skip > implicit
- recycle-32-freed: glibc > explicit > segregated > segregated-skip > implicit
- recycle-32-pinned: explicit > glibc > segregated > segregated-skip > implicit
- recycle-512-freed: glibc > explicit > segregated > segregated-skip > implicit
- recycle-512-pinned: explicit > glibc > segregated > segregated-skip > implicit
- recycle-64-freed: glibc > explicit > segregated > segregated-skip > implicit
- recycle-64-pinned: explicit > glibc > segregated > segregated-skip > implicit
- recycle-mixed-freed: glibc > explicit > segregated > segregated-skip > implicit
- recycle-mixed-pinned: glibc > segregated > explicit > segregated-skip > implicit

## fragmentation

| trace | version | Mops/s | vs glibc | scans/malloc | splits/malloc | peak util | rss MB |
|---|---|---|---|---|---|---|---|
| frag-grow | glibc | 26.97 | 1.000x | - | - | 0.519 | 13.4 |
| frag-grow | implicit | 0.02 | 0.001x | 20000.5 | 1.00 | 0.522 | 13.4 |
| frag-grow | explicit | 7.22 | 0.268x | 68.6 | 1.00 | 0.522 | 13.4 |
| frag-grow | segregated | 0.34 | 0.013x | 2247.7 | 1.00 | 0.522 | 13.4 |
| frag-grow | segregated-skip | 43.22 | 1.602x | 1.0 | 1.00 | 0.522 | 13.4 |

rankings (fastest first):

- frag-grow: segregated-skip > glibc > explicit > segregated > implicit

## size distributions

| trace | version | Mops/s | vs glibc | scans/malloc | splits/malloc | peak util | rss MB |
|---|---|---|---|---|---|---|---|
| dist-bimodal | glibc | 36.77 | 1.000x | - | - | 0.846 | 1.9 |
| dist-bimodal | implicit | 0.53 | 0.014x | 1106.9 | 0.43 | 0.872 | 2.0 |
| dist-bimodal | explicit | 38.78 | 1.055x | 3.6 | 0.95 | 0.620 | 2.8 |
| dist-bimodal | segregated | 20.54 | 0.559x | 11.0 | 0.49 | 0.821 | 2.1 |
| dist-bimodal | segregated-skip | 34.15 | 0.929x | 1.0 | 0.94 | 0.634 | 2.7 |
| dist-boundary | glibc | 39.16 | 1.000x | - | - | 0.906 | 3.7 |
| dist-boundary | implicit | 0.35 | 0.009x | 1124.3 | 0.32 | 0.891 | 3.9 |
| dist-boundary | explicit | 35.63 | 0.910x | 3.6 | 0.96 | 0.585 | 6.0 |
| dist-boundary | segregated | 15.44 | 0.394x | 14.9 | 0.46 | 0.851 | 4.1 |
| dist-boundary | segregated-skip | 31.45 | 0.803x | 1.0 | 0.86 | 0.677 | 5.2 |
| dist-classes | glibc | 98.51 | 1.000x | - | - | 0.789 | 0.7 |
| dist-classes | implicit | 0.48 | 0.005x | 1113.2 | 0.35 | 0.814 | 0.7 |
| dist-classes | explicit | 43.02 | 0.437x | 2.8 | 0.95 | 0.610 | 1.0 |
| dist-classes | segregated | 21.77 | 0.221x | 11.4 | 0.40 | 0.814 | 0.7 |
| dist-classes | segregated-skip | 30.63 | 0.311x | 1.0 | 0.95 | 0.574 | 1.0 |
| dist-loguniform | glibc | 27.02 | 1.000x | - | - | 0.851 | 5.8 |
| dist-loguniform | implicit | 0.35 | 0.013x | 1221.9 | 0.65 | 0.884 | 5.7 |
| dist-loguniform | explicit | 34.88 | 1.291x | 3.4 | 0.99 | 0.583 | 8.6 |
| dist-loguniform | segregated | 16.63 | 0.615x | 10.2 | 0.65 | 0.804 | 6.2 |
| dist-loguniform | segregated-skip | 24.61 | 0.911x | 1.0 | 0.96 | 0.644 | 7.8 |
| dist-small | glibc | 120.69 | 1.000x | - | - | 0.388 | 0.1 |
| dist-small | implicit | 0.79 | 0.007x | 1026.9 | 0.14 | 0.533 | 0.2 |
| dist-small | explicit | 59.40 | 0.492x | 1.9 | 0.78 | 0.400 | 0.2 |
| dist-small | segregated | 50.53 | 0.419x | 4.5 | 0.16 | 0.533 | 0.2 |
| dist-small | segregated-skip | 40.62 | 0.337x | 1.0 | 0.90 | 0.400 | 0.2 |

rankings (fastest first):

- dist-bimodal: explicit > glibc > segregated-skip > segregated > implicit
- dist-boundary: glibc > explicit > segregated-skip > segregated > implicit
- dist-classes: glibc > explicit > segregated-skip > segregated > implicit
- dist-loguniform: explicit > glibc > segregated-skip > segregated > implicit
- dist-small: glibc > explicit > segregated > segregated-skip > implicit

## hardware counters (profiling pass, user space, per op)

| trace | version | cycles | instr | IPC | branch miss | L1D miss | cache miss | dTLB miss | page faults |
|---|---|---|---|---|---|---|---|---|---|
| bimodal | implicit | 2114 | 2815 | 1.33 | 7.12 | 211.87 | 4.51 | 0.02 | 0.0000 |
| checkerboard | implicit | 96592 | 35466 | 0.37 | 0.81 | 3752.77 | 2101.79 | 0.08 | 0.0000 |
| dist-bimodal | implicit | 5025 | 5137 | 1.02 | 11.33 | 705.17 | 28.01 | 0.02 | 0.0000 |
| dist-boundary | implicit | 7692 | 5229 | 0.68 | 9.64 | 735.22 | 82.05 | 0.05 | 0.0000 |
| dist-classes | implicit | 5513 | 5167 | 0.94 | 10.26 | 681.03 | 22.56 | 0.03 | 0.0000 |
| dist-loguniform | implicit | 8072 | 5769 | 0.71 | 11.45 | 716.14 | 33.25 | 0.05 | 0.0000 |
| dist-small | implicit | 3356 | 4687 | 1.40 | 8.64 | 410.77 | 9.80 | 0.01 | 0.0000 |
| frag-grow | implicit | 153957 | 99549 | 0.65 | 1.03 | 14393.51 | 6015.70 | 269.53 | 0.0000 |
| grow-free-all | implicit | 21206 | 11299 | 0.53 | 0.77 | 1242.59 | 35.91 | 0.04 | 0.0000 |
| holes-16k | implicit | 51536 | 77490 | 1.50 | 0.56 | 6387.72 | 151.24 | 0.03 | 0.0000 |
| holes-1k | implicit | 6137 | 9549 | 1.56 | 0.78 | 797.87 | 8.17 | 0.00 | 0.0000 |
| holes-4k | implicit | 15427 | 23349 | 1.51 | 0.65 | 1898.54 | 100.85 | 0.01 | 0.0000 |
| huge-churn | implicit | 256 | 232 | 0.90 | 5.01 | 0.95 | 0.98 | 0.48 | 0.0000 |
| large-holes-2000 | implicit | 25123 | 10410 | 0.41 | 0.72 | 1115.21 | 107.11 | 194.38 | 0.0000 |
| large-holes-500 | implicit | 5771 | 3610 | 0.63 | 0.79 | 330.47 | 34.34 | 0.45 | 0.0000 |
| large-holes-8000 | implicit | 185361 | 37428 | 0.20 | 0.73 | 4127.39 | 3947.10 | 1987.73 | 0.0000 |
| random | implicit | 6342 | 3949 | 0.62 | 9.32 | 375.86 | 2.53 | 0.06 | 0.0000 |
| recycle-1024-freed | implicit | 2470 | 2490 | 1.01 | 3.53 | 313.16 | 78.43 | 0.01 | 0.0000 |
| recycle-1024-pinned | implicit | 7173 | 4532 | 0.63 | 0.50 | 467.70 | 0.40 | 0.00 | 0.0000 |
| recycle-128-freed | implicit | 2369 | 2421 | 1.02 | 4.18 | 257.66 | 52.67 | 0.00 | 0.0000 |
| recycle-128-pinned | implicit | 4412 | 4535 | 1.03 | 0.50 | 649.27 | 7.70 | 0.01 | 0.0000 |
| recycle-256-freed | implicit | 2285 | 2473 | 1.08 | 4.00 | 294.32 | 93.03 | 0.00 | 0.0000 |
| recycle-256-pinned | implicit | 6789 | 4545 | 0.67 | 0.50 | 468.66 | 0.22 | 0.00 | 0.0000 |
| recycle-32-freed | implicit | 1586 | 2314 | 1.46 | 0.51 | 100.01 | 1.51 | 0.00 | 0.0000 |
| recycle-32-pinned | implicit | 3112 | 4547 | 1.46 | 0.50 | 334.81 | 5.18 | 0.00 | 0.0000 |
| recycle-512-freed | implicit | 2404 | 2489 | 1.04 | 3.64 | 311.41 | 142.94 | 0.00 | 0.0000 |
| recycle-512-pinned | implicit | 6943 | 4545 | 0.65 | 0.50 | 467.97 | 0.28 | 0.00 | 0.0000 |
| recycle-64-freed | implicit | 1666 | 2350 | 1.41 | 2.45 | 266.57 | 5.31 | 0.00 | 0.0000 |
| recycle-64-pinned | implicit | 3132 | 4542 | 1.45 | 0.50 | 471.58 | 7.29 | 0.00 | 0.0000 |
| recycle-mixed-freed | implicit | 2858 | 2647 | 0.93 | 7.22 | 333.50 | 47.61 | 0.02 | 0.0000 |
| recycle-mixed-pinned | implicit | 5261 | 4954 | 0.94 | 7.36 | 612.78 | 52.78 | 0.03 | 0.0000 |
| scale-1k | implicit | 5413 | 3448 | 0.64 | 8.69 | 315.13 | 2.32 | 0.03 | 0.0000 |
| scale-2k | implicit | 11174 | 6753 | 0.60 | 9.98 | 691.88 | 4.51 | 0.06 | 0.0000 |
| scale-4k | implicit | 25094 | 13292 | 0.53 | 14.70 | 1406.69 | 83.81 | 17.47 | 0.0000 |
| small-fixed | implicit | 1151 | 1597 | 1.39 | 4.52 | 37.93 | 1.84 | 0.00 | 0.0000 |
| throughput | implicit | 3323 | 2526 | 0.76 | 8.80 | 220.19 | 2.82 | 0.03 | 0.0000 |
| bimodal | explicit | 75 | 74 | 0.99 | 1.03 | 4.16 | 0.45 | 0.06 | 0.0000 |
| checkerboard | explicit | 49 | 81 | 1.65 | 0.12 | 5.59 | 2.19 | 0.01 | 0.0000 |
| dist-bimodal | explicit | 70 | 75 | 1.07 | 0.68 | 5.10 | 1.42 | 0.02 | 0.0000 |
| dist-boundary | explicit | 75 | 75 | 1.00 | 0.66 | 4.97 | 1.62 | 0.03 | 0.0000 |
| dist-classes | explicit | 62 | 71 | 1.14 | 0.67 | 4.70 | 1.29 | 0.01 | 0.0000 |
| dist-loguniform | explicit | 77 | 75 | 0.97 | 0.66 | 5.02 | 1.81 | 0.09 | 0.0000 |
| dist-small | explicit | 44 | 66 | 1.48 | 0.60 | 3.33 | 0.23 | 0.00 | 0.0000 |
| frag-grow | explicit | 346 | 336 | 0.97 | 0.01 | 43.29 | 36.14 | 2.49 | 0.0000 |
| grow-free-all | explicit | 39 | 65 | 1.67 | 0.23 | 3.62 | 1.14 | 0.01 | 0.0000 |
| holes-16k | explicit | 163 | 127 | 0.78 | 0.02 | 14.49 | 7.02 | 0.01 | 0.0000 |
| holes-1k | explicit | 48 | 99 | 2.05 | 0.06 | 9.40 | 0.29 | 0.01 | 0.0000 |
| holes-4k | explicit | 82 | 118 | 1.44 | 0.05 | 13.07 | 1.88 | 0.01 | 0.0000 |
| huge-churn | explicit | 62 | 69 | 1.11 | 1.10 | 1.22 | 1.08 | 0.47 | 0.0000 |
| large-holes-2000 | explicit | 1208 | 970 | 0.80 | 0.12 | 115.31 | 250.36 | 59.12 | 0.0000 |
| large-holes-500 | explicit | 472 | 754 | 1.60 | 0.21 | 19.83 | 5.21 | 0.55 | 0.0000 |
| large-holes-8000 | explicit | 2025 | 1048 | 0.52 | 0.05 | 124.96 | 149.03 | 122.79 | 0.0000 |
| random | explicit | 84 | 78 | 0.93 | 1.25 | 4.34 | 1.02 | 0.03 | 0.0000 |
| recycle-1024-freed | explicit | 21 | 54 | 2.58 | 0.07 | 1.62 | 0.07 | 0.00 | 0.0000 |
| recycle-1024-pinned | explicit | 17 | 53 | 3.02 | 0.00 | 1.41 | 0.02 | 0.00 | 0.0000 |
| recycle-128-freed | explicit | 19 | 54 | 2.83 | 0.07 | 1.63 | 0.04 | 0.00 | 0.0000 |
| recycle-128-pinned | explicit | 16 | 53 | 3.32 | 0.00 | 1.41 | 0.03 | 0.00 | 0.0000 |
| recycle-256-freed | explicit | 20 | 54 | 2.74 | 0.07 | 1.63 | 0.04 | 0.00 | 0.0000 |
| recycle-256-pinned | explicit | 17 | 53 | 3.19 | 0.00 | 1.42 | 0.04 | 0.00 | 0.0000 |
| recycle-32-freed | explicit | 18 | 54 | 2.98 | 0.08 | 1.29 | 0.02 | 0.00 | 0.0000 |
| recycle-32-pinned | explicit | 15 | 53 | 3.49 | 0.00 | 1.28 | 0.01 | 0.00 | 0.0000 |
| recycle-512-freed | explicit | 21 | 54 | 2.58 | 0.07 | 1.63 | 0.07 | 0.00 | 0.0000 |
| recycle-512-pinned | explicit | 17 | 53 | 3.12 | 0.00 | 1.40 | 0.02 | 0.00 | 0.0000 |
| recycle-64-freed | explicit | 19 | 54 | 2.84 | 0.07 | 1.71 | 0.01 | 0.00 | 0.0000 |
| recycle-64-pinned | explicit | 17 | 53 | 3.05 | 0.00 | 1.42 | 0.03 | 0.00 | 0.0000 |
| recycle-mixed-freed | explicit | 51 | 68 | 1.33 | 0.65 | 4.15 | 0.51 | 0.01 | 0.0000 |
| recycle-mixed-pinned | explicit | 51 | 67 | 1.33 | 0.64 | 4.09 | 0.66 | 0.00 | 0.0000 |
| scale-128k | explicit | 6495 | 561 | 0.09 | 0.53 | 80.76 | 120.83 | 61.89 | 0.0000 |
| scale-16k | explicit | 332 | 109 | 0.33 | 0.66 | 9.93 | 6.69 | 4.06 | 0.0000 |
| scale-1k | explicit | 59 | 71 | 1.21 | 0.65 | 4.15 | 1.07 | 0.02 | 0.0000 |
| scale-2k | explicit | 77 | 76 | 0.99 | 0.67 | 4.86 | 1.68 | 0.02 | 0.0000 |
| scale-32k | explicit | 775 | 149 | 0.19 | 0.64 | 16.11 | 14.69 | 9.05 | 0.0000 |
| scale-4k | explicit | 109 | 81 | 0.75 | 0.67 | 5.60 | 2.88 | 0.55 | 0.0000 |
| scale-64k | explicit | 2178 | 253 | 0.12 | 0.60 | 32.60 | 40.62 | 22.63 | 0.0000 |
| scale-8k | explicit | 181 | 92 | 0.51 | 0.67 | 7.29 | 4.02 | 1.93 | 0.0000 |
| small-fixed | explicit | 50 | 64 | 1.27 | 1.13 | 1.77 | 0.01 | 0.00 | 0.0000 |
| throughput | explicit | 68 | 71 | 1.05 | 1.23 | 3.76 | 0.56 | 0.01 | 0.0000 |
| bimodal | segregated | 68 | 90 | 1.32 | 1.03 | 1.99 | 0.15 | 0.01 | 0.0000 |
| checkerboard | segregated | 62 | 101 | 1.63 | 0.12 | 3.71 | 2.00 | 0.01 | 0.0000 |
| dist-bimodal | segregated | 129 | 126 | 0.97 | 0.93 | 3.78 | 0.66 | 0.10 | 0.0000 |
| dist-boundary | segregated | 168 | 141 | 0.84 | 0.86 | 4.93 | 0.49 | 0.15 | 0.0000 |
| dist-classes | segregated | 120 | 125 | 1.04 | 0.76 | 3.83 | 0.40 | 0.09 | 0.0000 |
| dist-loguniform | segregated | 159 | 127 | 0.80 | 1.00 | 5.96 | 0.94 | 0.11 | 0.0000 |
| dist-small | segregated | 52 | 91 | 1.75 | 0.38 | 2.13 | 0.03 | 0.00 | 0.0000 |
| frag-grow | segregated | 7831 | 9089 | 1.16 | 0.39 | 1312.00 | 479.45 | 0.99 | 0.0000 |
| grow-free-all | segregated | 63 | 101 | 1.60 | 0.25 | 3.63 | 1.08 | 0.01 | 0.0000 |
| holes-16k | segregated | 56 | 101 | 1.79 | 0.02 | 3.28 | 1.45 | 0.01 | 0.0000 |
| holes-1k | segregated | 43 | 101 | 2.35 | 0.06 | 3.14 | 0.21 | 0.01 | 0.0000 |
| holes-4k | segregated | 49 | 101 | 2.05 | 0.07 | 3.34 | 0.88 | 0.01 | 0.0000 |
| huge-churn | segregated | 94 | 103 | 1.10 | 1.14 | 1.15 | 1.00 | 0.42 | 0.0000 |
| large-holes-2000 | segregated | 890 | 1009 | 1.13 | 0.09 | 115.42 | 131.06 | 59.96 | 0.0000 |
| large-holes-500 | segregated | 512 | 795 | 1.55 | 0.23 | 20.01 | 0.56 | 0.60 | 0.0000 |
| large-holes-8000 | segregated | 1979 | 1087 | 0.55 | 0.03 | 125.18 | 154.39 | 122.92 | 0.0000 |
| random | segregated | 177 | 151 | 0.85 | 1.52 | 4.65 | 0.67 | 0.14 | 0.0000 |
| recycle-1024-freed | segregated | 33 | 77 | 2.33 | 0.08 | 1.64 | 0.09 | 0.01 | 0.0000 |
| recycle-1024-pinned | segregated | 29 | 73 | 2.56 | 0.00 | 1.41 | 0.04 | 0.00 | 0.0000 |
| recycle-128-freed | segregated | 28 | 75 | 2.72 | 0.05 | 1.55 | 0.02 | 0.00 | 0.0000 |
| recycle-128-pinned | segregated | 23 | 73 | 3.16 | 0.00 | 1.42 | 0.03 | 0.00 | 0.0000 |
| recycle-256-freed | segregated | 32 | 76 | 2.38 | 0.09 | 1.62 | 0.06 | 0.00 | 0.0000 |
| recycle-256-pinned | segregated | 25 | 73 | 2.92 | 0.00 | 1.42 | 0.02 | 0.00 | 0.0000 |
| recycle-32-freed | segregated | 23 | 73 | 3.16 | 0.01 | 1.12 | 0.01 | 0.00 | 0.0000 |
| recycle-32-pinned | segregated | 23 | 73 | 3.17 | 0.00 | 1.29 | 0.02 | 0.00 | 0.0000 |
| recycle-512-freed | segregated | 32 | 76 | 2.42 | 0.08 | 1.62 | 0.06 | 0.00 | 0.0000 |
| recycle-512-pinned | segregated | 27 | 73 | 2.72 | 0.00 | 1.42 | 0.03 | 0.00 | 0.0000 |
| recycle-64-freed | segregated | 37 | 76 | 2.03 | 0.05 | 1.55 | 0.01 | 0.00 | 0.0000 |
| recycle-64-pinned | segregated | 23 | 73 | 3.25 | 0.00 | 1.42 | 0.02 | 0.00 | 0.0000 |
| recycle-mixed-freed | segregated | 52 | 85 | 1.64 | 0.43 | 1.86 | 0.07 | 0.01 | 0.0000 |
| recycle-mixed-pinned | segregated | 35 | 74 | 2.14 | 0.08 | 1.87 | 0.11 | 0.01 | 0.0000 |
| scale-128k | segregated | 35527 | 2988 | 0.08 | 0.76 | 457.52 | 660.87 | 357.34 | 0.0000 |
| scale-16k | segregated | 2717 | 758 | 0.28 | 0.98 | 107.83 | 7.44 | 19.15 | 0.0000 |
| scale-1k | segregated | 169 | 152 | 0.90 | 1.05 | 4.44 | 0.76 | 0.15 | 0.0000 |
| scale-2k | segregated | 271 | 196 | 0.72 | 1.03 | 7.32 | 1.19 | 0.18 | 0.0000 |
| scale-32k | segregated | 7137 | 1284 | 0.18 | 0.93 | 190.09 | 56.02 | 86.79 | 0.0000 |
| scale-4k | segregated | 519 | 282 | 0.54 | 1.01 | 17.29 | 1.95 | 0.43 | 0.0000 |
| scale-64k | segregated | 19154 | 2086 | 0.11 | 0.86 | 315.79 | 285.01 | 233.86 | 0.0000 |
| scale-8k | segregated | 1184 | 454 | 0.38 | 1.00 | 50.13 | 2.97 | 2.47 | 0.0000 |
| small-fixed | segregated | 52 | 81 | 1.55 | 0.86 | 1.18 | 0.02 | 0.00 | 0.0000 |
| throughput | segregated | 121 | 130 | 1.07 | 1.55 | 3.14 | 0.37 | 0.03 | 0.0000 |
| bimodal | glibc | 61 | 78 | 1.28 | 1.10 | 1.93 | 0.24 | 0.04 | 0.0013 |
| checkerboard | glibc | 71 | 167 | 2.36 | 0.33 | 4.19 | 2.82 | 0.01 | 0.0000 |
| dist-bimodal | glibc | 72 | 106 | 1.47 | 1.23 | 2.63 | 0.41 | 0.03 | 0.0000 |
| dist-boundary | glibc | 66 | 100 | 1.52 | 1.08 | 2.16 | 0.19 | 0.01 | 0.0002 |
| dist-classes | glibc | 27 | 55 | 2.05 | 0.10 | 1.52 | 0.13 | 0.01 | 0.0000 |
| dist-loguniform | glibc | 99 | 123 | 1.25 | 1.67 | 3.56 | 0.58 | 0.04 | 0.0000 |
| dist-small | glibc | 22 | 54 | 2.47 | 0.07 | 1.37 | 0.08 | 0.00 | 0.0000 |
| frag-grow | glibc | 77 | 167 | 2.18 | 0.24 | 4.33 | 3.63 | 0.34 | 0.0035 |
| grow-free-all | glibc | 122 | 192 | 1.58 | 2.00 | 4.64 | 1.65 | 0.03 | 0.0000 |
| holes-16k | glibc | 66 | 175 | 2.66 | 0.09 | 3.35 | 1.51 | 0.00 | 0.0000 |
| holes-1k | glibc | 70 | 228 | 3.26 | 0.32 | 3.67 | 0.55 | 0.01 | 0.0006 |
| holes-4k | glibc | 65 | 192 | 2.97 | 0.19 | 3.55 | 1.02 | 0.00 | 0.0000 |
| huge-churn | glibc | 572 | 315 | 0.55 | 4.00 | 16.95 | 2.77 | 0.94 | 0.2929 |
| large-holes-2000 | glibc | 127 | 258 | 2.03 | 0.62 | 4.17 | 1.13 | 0.54 | 0.0410 |
| large-holes-500 | glibc | 229 | 240 | 1.05 | 1.66 | 8.39 | 1.57 | 0.85 | 0.1567 |
| large-holes-8000 | glibc | 173 | 278 | 1.61 | 0.45 | 4.79 | 2.77 | 0.96 | 0.0414 |
| random | glibc | 139 | 182 | 1.31 | 2.86 | 3.88 | 0.52 | 0.06 | 0.0000 |
| recycle-1024-freed | glibc | 19 | 50 | 2.67 | 0.01 | 1.01 | 0.04 | 0.00 | 0.0000 |
| recycle-1024-pinned | glibc | 20 | 50 | 2.56 | 0.01 | 1.06 | 0.03 | 0.00 | 0.0000 |
| recycle-128-freed | glibc | 33 | 49 | 1.47 | 0.00 | 1.07 | 0.02 | 0.00 | 0.0000 |
| recycle-128-pinned | glibc | 16 | 49 | 3.06 | 0.00 | 1.01 | 0.01 | 0.00 | 0.0000 |
| recycle-256-freed | glibc | 17 | 49 | 2.92 | 0.00 | 1.07 | 0.03 | 0.00 | 0.0000 |
| recycle-256-pinned | glibc | 17 | 49 | 2.84 | 0.00 | 1.03 | 0.04 | 0.00 | 0.0000 |
| recycle-32-freed | glibc | 16 | 49 | 3.09 | 0.00 | 1.08 | 0.01 | 0.00 | 0.0000 |
| recycle-32-pinned | glibc | 16 | 49 | 3.11 | 0.00 | 1.05 | 0.01 | 0.00 | 0.0000 |
| recycle-512-freed | glibc | 18 | 49 | 2.73 | 0.00 | 1.07 | 0.04 | 0.00 | 0.0000 |
| recycle-512-pinned | glibc | 18 | 49 | 2.72 | 0.01 | 1.03 | 0.03 | 0.00 | 0.0000 |
| recycle-64-freed | glibc | 16 | 49 | 3.09 | 0.00 | 1.01 | 0.01 | 0.00 | 0.0000 |
| recycle-64-pinned | glibc | 16 | 49 | 3.08 | 0.00 | 1.00 | 0.01 | 0.00 | 0.0000 |
| recycle-mixed-freed | glibc | 25 | 56 | 2.24 | 0.11 | 1.30 | 0.10 | 0.01 | 0.0000 |
| recycle-mixed-pinned | glibc | 24 | 55 | 2.27 | 0.09 | 1.32 | 0.12 | 0.00 | 0.0000 |
| scale-128k | glibc | 379 | 210 | 0.56 | 1.73 | 5.08 | 4.65 | 1.73 | 0.0001 |
| scale-16k | glibc | 193 | 182 | 0.95 | 2.13 | 4.76 | 3.28 | 1.13 | 0.0000 |
| scale-1k | glibc | 120 | 181 | 1.51 | 2.16 | 3.77 | 0.52 | 0.04 | 0.0000 |
| scale-2k | glibc | 131 | 181 | 1.38 | 2.20 | 4.16 | 0.92 | 0.04 | 0.0000 |
| scale-32k | glibc | 207 | 186 | 0.90 | 2.04 | 4.86 | 3.72 | 1.36 | 0.0000 |
| scale-4k | glibc | 158 | 181 | 1.15 | 2.22 | 4.40 | 1.91 | 0.19 | 0.0000 |
| scale-64k | glibc | 260 | 195 | 0.75 | 1.91 | 4.97 | 4.20 | 1.55 | 0.0001 |
| scale-8k | glibc | 178 | 181 | 1.01 | 2.19 | 4.63 | 2.69 | 0.75 | 0.0000 |
| small-fixed | glibc | 34 | 55 | 1.63 | 0.58 | 0.98 | 0.03 | 0.00 | 0.0000 |
| throughput | glibc | 30 | 49 | 1.64 | 0.52 | 1.27 | 0.05 | 0.01 | 0.0000 |
| bimodal | segregated-skip | 79 | 97 | 1.22 | 1.11 | 3.08 | 0.24 | 0.02 | 0.0000 |
| checkerboard | segregated-skip | 66 | 101 | 1.52 | 0.12 | 3.70 | 2.01 | 0.01 | 0.0000 |
| dist-bimodal | segregated-skip | 96 | 98 | 1.02 | 0.56 | 4.00 | 0.90 | 0.00 | 0.0000 |
| dist-boundary | segregated-skip | 89 | 96 | 1.07 | 0.47 | 3.65 | 0.77 | 0.01 | 0.0000 |
| dist-classes | segregated-skip | 87 | 98 | 1.12 | 0.44 | 3.80 | 0.66 | 0.01 | 0.0000 |
| dist-loguniform | segregated-skip | 102 | 98 | 0.97 | 0.44 | 3.88 | 0.91 | 0.03 | 0.0000 |
| dist-small | segregated-skip | 75 | 97 | 1.28 | 0.46 | 3.42 | 0.19 | 0.00 | 0.0000 |
| frag-grow | segregated-skip | 62 | 101 | 1.65 | 0.01 | 3.99 | 2.60 | 0.31 | 0.0000 |
| grow-free-all | segregated-skip | 77 | 100 | 1.30 | 0.25 | 3.63 | 1.16 | 0.01 | 0.0000 |
| holes-16k | segregated-skip | 56 | 101 | 1.80 | 0.02 | 3.34 | 1.58 | 0.01 | 0.0000 |
| holes-1k | segregated-skip | 42 | 101 | 2.40 | 0.06 | 3.13 | 0.19 | 0.01 | 0.0000 |
| holes-4k | segregated-skip | 53 | 101 | 1.92 | 0.07 | 3.34 | 0.93 | 0.01 | 0.0000 |
| huge-churn | segregated-skip | 92 | 102 | 1.11 | 1.16 | 1.12 | 0.97 | 0.34 | 0.0000 |
| large-holes-2000 | segregated-skip | 960 | 1009 | 1.05 | 0.09 | 115.32 | 155.41 | 59.40 | 0.0000 |
| large-holes-500 | segregated-skip | 489 | 795 | 1.63 | 0.21 | 25.12 | 11.10 | 0.60 | 0.0000 |
| large-holes-8000 | segregated-skip | 1964 | 1087 | 0.55 | 0.03 | 125.17 | 173.57 | 122.95 | 0.0000 |
| random | segregated-skip | 88 | 99 | 1.13 | 1.00 | 3.77 | 0.71 | 0.02 | 0.0000 |
| recycle-1024-freed | segregated-skip | 67 | 99 | 1.47 | 0.44 | 3.03 | 0.14 | 0.00 | 0.0000 |
| recycle-1024-pinned | segregated-skip | 72 | 99 | 1.38 | 0.44 | 2.98 | 0.03 | 0.00 | 0.0000 |
| recycle-128-freed | segregated-skip | 67 | 99 | 1.49 | 0.45 | 3.29 | 0.03 | 0.00 | 0.0000 |
| recycle-128-pinned | segregated-skip | 60 | 99 | 1.65 | 0.43 | 3.27 | 0.06 | 0.00 | 0.0000 |
| recycle-256-freed | segregated-skip | 66 | 99 | 1.51 | 0.45 | 3.18 | 0.08 | 0.00 | 0.0000 |
| recycle-256-pinned | segregated-skip | 63 | 99 | 1.59 | 0.44 | 3.11 | 0.07 | 0.00 | 0.0000 |
| recycle-32-freed | segregated-skip | 72 | 100 | 1.37 | 0.45 | 2.41 | 0.07 | 0.00 | 0.0000 |
| recycle-32-pinned | segregated-skip | 71 | 100 | 1.40 | 0.44 | 2.42 | 0.01 | 0.00 | 0.0000 |
| recycle-512-freed | segregated-skip | 80 | 99 | 1.24 | 0.44 | 3.03 | 0.14 | 0.00 | 0.0000 |
| recycle-512-pinned | segregated-skip | 76 | 99 | 1.30 | 0.43 | 2.99 | 0.09 | 0.00 | 0.0000 |
| recycle-64-freed | segregated-skip | 69 | 99 | 1.43 | 0.46 | 3.70 | 0.07 | 0.00 | 0.0000 |
| recycle-64-pinned | segregated-skip | 60 | 99 | 1.66 | 0.43 | 3.72 | 0.10 | 0.00 | 0.0000 |
| recycle-mixed-freed | segregated-skip | 74 | 98 | 1.33 | 0.47 | 3.39 | 0.35 | 0.00 | 0.0000 |
| recycle-mixed-pinned | segregated-skip | 88 | 98 | 1.11 | 0.46 | 3.39 | 0.35 | 0.00 | 0.0000 |
| scale-128k | segregated-skip | 354 | 101 | 0.28 | 0.44 | 4.43 | 5.34 | 2.07 | 0.0000 |
| scale-16k | segregated-skip | 266 | 100 | 0.37 | 0.47 | 4.52 | 4.14 | 1.70 | 0.0000 |
| scale-1k | segregated-skip | 102 | 99 | 0.97 | 0.53 | 3.68 | 0.72 | 0.00 | 0.0000 |
| scale-2k | segregated-skip | 113 | 99 | 0.88 | 0.51 | 4.07 | 1.13 | 0.01 | 0.0000 |
| scale-32k | segregated-skip | 291 | 100 | 0.34 | 0.47 | 4.51 | 4.76 | 1.94 | 0.0000 |
| scale-4k | segregated-skip | 142 | 99 | 0.70 | 0.49 | 4.34 | 2.40 | 0.56 | 0.0000 |
| scale-64k | segregated-skip | 307 | 100 | 0.33 | 0.46 | 4.47 | 5.14 | 2.10 | 0.0000 |
| scale-8k | segregated-skip | 195 | 99 | 0.51 | 0.48 | 4.47 | 3.29 | 0.37 | 0.0000 |
| small-fixed | segregated-skip | 71 | 97 | 1.36 | 1.04 | 2.19 | 0.10 | 0.00 | 0.0000 |
| throughput | segregated-skip | 80 | 99 | 1.23 | 1.01 | 3.48 | 0.44 | 0.01 | 0.0000 |

## memory: live, logical heap, reserved, resident

driver rss = clear_refs/VmHWM inside the process; independent = child maxrss (wait4) minus a no-allocator baseline run

| trace | version | peak live MB | heap MB | reserved MB | rss (driver) MB | rss (independent) MB |
|---|---|---|---|---|---|---|
| small-fixed | implicit | 0.0 | 0.1 | 4096.0 | 0.1 | 0.0 |
| random | implicit | 2.6 | 2.9 | 4096.0 | 2.9 | 3.0 |
| bimodal | implicit | 5.5 | 6.4 | 4096.0 | 6.3 | 6.3 |
| throughput | implicit | 0.5 | 0.6 | 4096.0 | 0.6 | 0.6 |
| grow-free-all | implicit | 5.0 | 5.1 | 4096.0 | 5.1 | 5.0 |
| checkerboard | implicit | 3.7 | 4.1 | 4096.0 | 4.0 | 4.1 |
| scale-1k | implicit | 2.1 | 2.4 | 4096.0 | 2.3 | 2.2 |
| scale-4k | implicit | 8.0 | 8.9 | 4096.0 | 8.9 | 8.8 |
| huge-churn | implicit | 152.3 | 178.0 | 4096.0 | 178.0 | 177.8 |
| large-holes-2000 | implicit | 78.2 | 127.1 | 4096.0 | 127.1 | 126.9 |
| dist-loguniform | implicit | 5.0 | 5.7 | 4096.0 | 5.7 | 5.6 |
| small-fixed | explicit | 0.0 | 0.1 | 4096.0 | 0.1 | 0.0 |
| random | explicit | 2.6 | 3.4 | 4096.0 | 3.4 | 3.2 |
| bimodal | explicit | 5.5 | 11.7 | 4096.0 | 11.7 | 11.8 |
| throughput | explicit | 0.5 | 0.7 | 4096.0 | 0.7 | 0.9 |
| grow-free-all | explicit | 5.0 | 5.1 | 4096.0 | 5.1 | 4.9 |
| checkerboard | explicit | 3.7 | 4.1 | 4096.0 | 4.0 | 4.0 |
| scale-1k | explicit | 2.1 | 2.9 | 4096.0 | 2.9 | 2.8 |
| scale-4k | explicit | 8.0 | 10.9 | 4096.0 | 10.9 | 10.7 |
| scale-16k | explicit | 31.8 | 42.8 | 4096.0 | 42.8 | 42.9 |
| scale-64k | explicit | 125.3 | 169.9 | 4096.0 | 169.9 | 169.7 |
| scale-128k | explicit | 250.7 | 336.5 | 4096.0 | 336.5 | 336.6 |
| huge-churn | explicit | 152.3 | 198.3 | 4096.0 | 198.0 | 197.8 |
| large-holes-2000 | explicit | 78.2 | 127.1 | 4096.0 | 127.1 | 126.9 |
| dist-loguniform | explicit | 5.0 | 8.6 | 4096.0 | 8.6 | 8.5 |
| small-fixed | segregated | 0.0 | 0.1 | 4096.0 | 0.1 | 0.1 |
| random | segregated | 2.6 | 3.0 | 4096.0 | 3.0 | 2.8 |
| bimodal | segregated | 5.5 | 7.1 | 4096.0 | 7.1 | 7.0 |
| throughput | segregated | 0.5 | 0.6 | 4096.0 | 0.6 | 0.6 |
| grow-free-all | segregated | 5.0 | 5.1 | 4096.0 | 5.1 | 5.1 |
| checkerboard | segregated | 3.7 | 4.1 | 4096.0 | 4.0 | 4.0 |
| scale-1k | segregated | 2.1 | 2.4 | 4096.0 | 2.4 | 2.5 |
| scale-4k | segregated | 8.0 | 9.4 | 4096.0 | 9.4 | 9.0 |
| scale-16k | segregated | 31.8 | 36.8 | 4096.0 | 36.8 | 36.8 |
| scale-64k | segregated | 125.3 | 145.1 | 4096.0 | 145.1 | 145.2 |
| scale-128k | segregated | 250.7 | 289.1 | 4096.0 | 289.1 | 289.3 |
| huge-churn | segregated | 152.3 | 198.3 | 4096.0 | 198.0 | 198.0 |
| large-holes-2000 | segregated | 78.2 | 127.1 | 4096.0 | 127.1 | 127.1 |
| dist-loguniform | segregated | 5.0 | 6.2 | 4096.0 | 6.2 | 6.2 |
| small-fixed | glibc | 0.0 | 0.1 | 0.0 | 0.0 | 0.1 |
| random | glibc | 2.6 | 3.1 | 3.0 | 3.0 | 3.1 |
| bimodal | glibc | 5.5 | 6.4 | 6.2 | 6.2 | 6.4 |
| throughput | glibc | 0.5 | 0.9 | 0.8 | 0.8 | 0.9 |
| grow-free-all | glibc | 5.0 | 5.2 | 5.0 | 5.0 | 4.9 |
| checkerboard | glibc | 3.7 | 4.1 | 4.0 | 4.0 | 3.9 |
| scale-1k | glibc | 2.1 | 2.6 | 2.5 | 2.5 | 2.6 |
| scale-4k | glibc | 8.0 | 8.9 | 8.8 | 8.8 | 8.7 |
| scale-16k | glibc | 31.8 | 33.3 | 33.1 | 33.1 | 33.2 |
| scale-64k | glibc | 125.3 | 128.9 | 128.8 | 128.6 | 128.8 |
| scale-128k | glibc | 250.7 | 255.8 | 255.6 | 255.4 | 255.6 |
| huge-churn | glibc | 152.3 | 179.5 | 179.4 | 179.1 | 179.2 |
| large-holes-2000 | glibc | 78.2 | 123.9 | 123.8 | 123.7 | 123.9 |
| dist-loguniform | glibc | 5.0 | 5.9 | 5.8 | 5.7 | 5.9 |
| small-fixed | segregated-skip | 0.0 | 0.1 | 4096.0 | 0.1 | 0.2 |
| random | segregated-skip | 2.6 | 3.8 | 4096.0 | 3.8 | 3.9 |
| bimodal | segregated-skip | 5.5 | 7.6 | 4096.0 | 7.6 | 7.6 |
| throughput | segregated-skip | 0.5 | 0.8 | 4096.0 | 0.8 | 0.8 |
| grow-free-all | segregated-skip | 5.0 | 5.1 | 4096.0 | 5.1 | 5.2 |
| checkerboard | segregated-skip | 3.7 | 4.1 | 4096.0 | 4.0 | 4.0 |
| scale-1k | segregated-skip | 2.1 | 3.1 | 4096.0 | 3.1 | 3.2 |
| scale-4k | segregated-skip | 8.0 | 11.6 | 4096.0 | 11.6 | 11.6 |
| scale-16k | segregated-skip | 31.8 | 45.6 | 4096.0 | 45.5 | 45.6 |
| scale-64k | segregated-skip | 125.3 | 180.8 | 4096.0 | 180.7 | 180.8 |
| scale-128k | segregated-skip | 250.7 | 357.8 | 4096.0 | 357.7 | 357.6 |
| huge-churn | segregated-skip | 152.3 | 198.3 | 4096.0 | 198.0 | 197.8 |
| large-holes-2000 | segregated-skip | 78.2 | 127.1 | 4096.0 | 127.1 | 127.0 |
| dist-loguniform | segregated-skip | 5.0 | 7.8 | 4096.0 | 7.8 | 8.0 |
