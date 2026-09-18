#!/usr/bin/env python3
"""Render docs/figures/bench.png from the CSV produced by `bench`.

    build/bench > bench.csv
    python3 bench/plot.py bench.csv docs/figures/bench.png
"""
import csv
import sys

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

src = sys.argv[1] if len(sys.argv) > 1 else "bench.csv"
dst = sys.argv[2] if len(sys.argv) > 2 else "docs/figures/bench.png"

times, gaps = [], []
for row in csv.reader(open(src)):
    if not row or row[0] in ("experiment", ""):
        continue
    if row[0] == "time":
        times.append((int(row[1]), int(float(row[2])), float(row[3]), float(row[4])))
    elif row[0] == "gap":
        gaps.append((int(row[1]), float(row[3]), float(row[4]), float(row[5])))

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(11, 4.3), dpi=130)

n = [t[0] for t in times]
ax1.loglog(n, [t[2] for t in times], "o-", color="#2563eb", label="fractional — greedy  O(n log n)")
ax1.loglog(n, [t[3] for t in times], "s-", color="#dc2626", label="0/1 — dynamic programming  O(n·W)")
ax1.set_xlabel("number of items n   (W = ¼ Σ wᵢ, wᵢ ≤ 1000)")
ax1.set_ylabel("time per instance (ms)")
ax1.set_title("Running time")
ax1.grid(alpha=0.3, which="both")
ax1.legend(fontsize=8)

g_n = [g[0] for g in gaps]
ax2.semilogx(g_n, [g[1] for g in gaps], "o-", color="#059669", label="mean gap")
ax2.semilogx(g_n, [g[2] for g in gaps], "^--", color="#d97706", label="max gap (300 instances)")
ax2.set_yscale("symlog", linthresh=1)
ax2.set_xlabel("number of items n")
ax2.set_ylabel("(fractional − 0/1 optimum) / 0/1 optimum  (%)")
ax2.set_title("Integrality gap of the fractional relaxation")
ax2.grid(alpha=0.3, which="both")
ax2.legend(fontsize=8)

fig.tight_layout()
fig.savefig(dst)
print(f"wrote {dst}")
