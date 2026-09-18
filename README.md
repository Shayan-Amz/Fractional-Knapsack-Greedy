# Fractional Knapsack — Greedy Algorithm

[![CI](https://github.com/Shayan-Amz/Fractional-Knapsack-Greedy/actions/workflows/ci.yml/badge.svg)](https://github.com/Shayan-Amz/Fractional-Knapsack-Greedy/actions/workflows/ci.yml)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white)](include/knapsack/fractional.hpp)
[![Header-only](https://img.shields.io/badge/library-header--only-blue)](include/knapsack/fractional.hpp)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

An `O(n log n)` greedy solver for the **fractional (continuous) knapsack problem**, packaged as a
small header-only C++17 library with a command-line front end, a proof of optimality, a test suite
that checks the greedy answer against exhaustive enumeration, and a benchmark that contrasts it with
the 0/1 variant — the classic example of where the same greedy idea *fails*.

The project originates from an Algorithms course assignment ("Juice Happiness": fill a stomach of
`V` litres from `n` juices, each with a volume and a happiness, drinking any fraction of a juice).

<p align="center">
  <img src="docs/figures/bench.png" alt="Running time and integrality gap" width="900">
  <br>
  <sub>Left: greedy fractional solver vs. 0/1 dynamic programming. Right: how far the fractional optimum lies above the 0/1 optimum on random instances.</sub>
</p>

---

## Table of Contents

1. [Problem](#problem)
2. [Algorithm](#algorithm)
3. [Proof of optimality](#proof-of-optimality)
4. [Why the same greedy fails for 0/1 knapsack](#why-the-same-greedy-fails-for-01-knapsack)
5. [Experiments](#experiments)
6. [Usage](#usage)
7. [Testing](#testing)
8. [Project structure](#project-structure)
9. [History](#history)
10. [References](#references)
11. [License](#license)

---

## Problem

Given `n` items with weights `w_i > 0`, values `v_i ≥ 0`, and a knapsack of capacity `W`:

```
maximise    Σ  v_i · x_i
subject to  Σ  w_i · x_i ≤ W,        0 ≤ x_i ≤ 1        (x_i ∈ ℝ)
```

`x_i` is the fraction of item `i` that is taken. Because the variables are continuous this is a
linear program, and — unlike the NP-hard 0/1 version where `x_i ∈ {0, 1}` — it has a simple exact
greedy solution.

---

## Algorithm

1. Compute the **value density** `ρ_i = v_i / w_i` of every item.
2. Sort the items by non-increasing density — `O(n log n)`.
3. Scan in that order: take each item whole while it fits; the first item that does not fit is
   taken **fractionally** so that the knapsack becomes exactly full, and the scan stops.

```
sort items by v_i / w_i  (descending)
remaining ← W
for each item i in that order:
    if w_i ≤ remaining:   x_i ← 1;  remaining ← remaining − w_i
    else:                 x_i ← remaining / w_i;  stop
```

*Time* `O(n log n)` (sorting dominates; the scan is `O(n)`). *Extra space* `O(n)` for the
permutation and the solution vector. The implementation compares densities as cross-products
(`v_a · w_b > v_b · w_a`) rather than quotients, which avoids a division per comparison and the
rounding it would introduce, and breaks ties by index so the output is deterministic.

---

## Proof of optimality

**Claim.** The greedy solution `x^G` attains the maximum.

**Proof (exchange argument).** Relabel the items so that `ρ_1 ≥ ρ_2 ≥ … ≥ ρ_n`. The greedy solution
has the form `x^G = (1, …, 1, f, 0, …, 0)` with the fractional entry `f ∈ [0, 1)` at some position `k`
(or is all ones if everything fits, in which case it is trivially optimal).

Let `x*` be an optimal solution that agrees with `x^G` on the longest possible prefix, and suppose
`x* ≠ x^G`. Let `i` be the first index where they differ.

* If `i < k`, then `x*_i < 1 = x^G_i`. Since `x*` is feasible and uses no more capacity than `x^G`
  up to `i`, and `x^G` fills the knapsack, `x*` must take strictly positive amounts of some items
  `j > i` (otherwise its total weight would be below `W` while an item with `x*_i < 1` is available,
  contradicting optimality unless `v_i = 0`, in which case swapping is free anyway).
* If `i = k`, then `x*_k < f` and, likewise, `x*` must place weight on items `j > k`.

In both cases move an amount of weight `δ > 0` from such an item `j` to item `i`
(`x*_i += δ / w_i`, `x*_j −= δ / w_j`, with `δ` small enough to keep both within `[0, 1]`).
Feasibility is unchanged and the objective changes by

```
Δ = δ · (v_i / w_i − v_j / w_j) = δ · (ρ_i − ρ_j) ≥ 0        because i < j ⇒ ρ_i ≥ ρ_j.
```

So the modified solution is still optimal and agrees with `x^G` on a longer prefix — contradicting
the choice of `x*`. Hence `x* = x^G`. ∎

The argument also shows the two structural properties the test-suite checks on every random instance:
an optimal solution exists with **at most one fractional item**, and the knapsack is **filled
completely** whenever the items do not all fit.

---

## Why the same greedy fails for 0/1 knapsack

If items are indivisible the density-greedy rule can be arbitrarily bad:

| item | weight | value | density |
|:----:|:------:|:-----:|:-------:|
| A | 1 | 2 | 2.0 |
| B | 10 | 10 | 1.0 |

With `W = 10`, greedy takes A first (highest density), then B no longer fits → value **2**.
The optimum takes B alone → value **10**. Scaling B's weight and value to `M` makes the ratio
`M / 2` unbounded.

The fractional relaxation is still useful for the 0/1 problem, though: its optimum is an **upper
bound** on the 0/1 optimum (relaxing constraints can only increase the maximum), which is exactly
the bound branch-and-bound solvers use to prune the search tree
(`knapsack::fractional_upper_bound`). The right-hand plot above measures how tight that bound is.

---

## Experiments

`bench/bench.cpp` (`make bench`, results on a typical x86-64 laptop core, `-O2`):

**Running time** — random instances, `w_i, v_i ∈ [1, 1000]`, `W = ¼ Σ w_i`:

| n | greedy fractional | 0/1 dynamic programming `O(n·W)` |
|--:|------------------:|---------------------------------:|
| 100 | 0.002 ms | 0.7 ms |
| 1 000 | 0.03 ms | 63 ms |
| 10 000 | 1.0 ms | 6.6 s |
| 20 000 | 2.1 ms | 26 s |

The DP is *pseudo-polynomial*: its cost grows with the numeric size of `W`, not just with `n`, which
is why the red line has slope ≈ 2 on the log-log plot (`W` grows with `n` here) while the greedy
line stays close to linear.

**Integrality gap** — 300 random instances per size, `w_i, v_i ∈ [1, 100]`:

| n | mean gap | max gap | greedy solution already integral |
|--:|---------:|--------:|---------------------------------:|
| 5 | 30 % | 374 % | 3 % |
| 20 | 2.6 % | 8.9 % | 2 % |
| 100 | 0.19 % | 0.53 % | 0.3 % |
| 200 | 0.05 % | 0.13 % | 2 % |

The gap is at most the value of the single fractional item, so it vanishes relative to the total as
`n` grows — the reason the fractional bound is so effective for pruning on large 0/1 instances, and
so weak on tiny ones.

---

## Usage

### Build

```bash
git clone https://github.com/Shayan-Amz/Fractional-Knapsack-Greedy.git
cd Fractional-Knapsack-Greedy
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

or, without CMake, `make && make test`. The library is a single header:
copy `include/knapsack/fractional.hpp` into any C++17 project.

### Command line

Instance format: first line `n W`, then `n` lines of `weight value`.

```
$ cat examples/clrs.txt
3 50
10 60
20 100
30 120

$ build/knapsack examples/clrs.txt
item       weight      value    density   fraction      taken
1              10         60     6.0000     1.0000         10
2              20        100     5.0000     1.0000         20
3              30        120     4.0000     0.6667         20

capacity used : 50 of 50
maximum value : 240
```

`--json` prints a machine-readable result, `--interactive` prompts for the input as the original
course program did, and an instance can also be piped on stdin.

### Library

```cpp
#include "knapsack/fractional.hpp"

std::vector<knapsack::Item> items = {{10, 60}, {20, 100}, {30, 120}};   // {weight, value}
knapsack::Solution s = knapsack::solve(items, 50);

s.total_value;    // 240
s.fraction;       // {1, 1, 0.6667}  — in the original item order
s.order;          // {0, 1, 2}       — items by decreasing density

double ub = knapsack::fractional_upper_bound(items, 50);   // bound for 0/1 branch-and-bound
double dp = knapsack::zero_one_dp(items, 50);              // 0/1 optimum (integer weights) = 220
```

Malformed instances (non-positive weight, negative value or capacity) raise `std::invalid_argument`.

---

## Testing

`tests/test_fractional.cpp` — no framework needed, runs in a quarter of a second:

* textbook instances (CLRS §16.2, the original assignment's example, all-fits, zero capacity, ties,
  extreme magnitudes, invalid input);
* **20 000 random instances** (`n ≤ 10`) whose greedy value is compared with an **exhaustive
  enumeration of the LP's vertices** — every vertex of the feasible polytope has at most one
  fractional coordinate, so "every subset taken whole + at most one extra item taken fractionally"
  is a complete search;
* on every instance: feasibility, at most one fractional item, capacity exhausted when the items
  do not all fit, non-increasing density order, and `fractional optimum ≥ 0/1 optimum` (with equality
  whenever the greedy solution is integral).

CI runs the suite with GCC, Clang, MSVC and on macOS.

---

## Project structure

```
.
├── include/knapsack/fractional.hpp   solve(), max_value(), zero_one_dp(), fractional_upper_bound()
├── src/main.cpp                      command-line solver (file / stdin / --json / --interactive)
├── tests/test_fractional.cpp         unit + randomized tests vs. exhaustive enumeration
├── bench/bench.cpp · bench/plot.py   timing and integrality-gap experiments → docs/figures/bench.png
├── examples/*.txt                    sample instances (CLRS, original assignment, Wikipedia)
├── CMakeLists.txt · Makefile · .github/workflows/ci.yml
└── LICENSE (MIT)
```

---

## History

The first version was a single 77-line file written for the course. This release keeps its
algorithm and fixes what was wrong around it: the prompt asked for *volume then happiness* but the
values were stored the other way round (so the documented example printed 166.7 instead of 0.6);
the array was released with `delete` instead of `delete[]`; a zero-volume item divided by zero; and
only the optimal value — not the plan — was reported.

---

## References

1. T. H. Cormen, C. E. Leiserson, R. L. Rivest, C. Stein, *Introduction to Algorithms*, 4th ed.,
   §16.2 "Elements of the greedy strategy" (fractional vs. 0-1 knapsack).
2. G. B. Dantzig, "Discrete-Variable Extremum Problems", *Operations Research* 5(2), 1957 —
   the origin of the density-ordering bound.
3. H. Kellerer, U. Pferschy, D. Pisinger, *Knapsack Problems*, Springer, 2004.

---

## License

Released under the [MIT License](LICENSE).
