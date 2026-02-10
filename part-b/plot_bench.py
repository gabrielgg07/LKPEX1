#!/usr/bin/env python3
"""plot_bench.py - Plot kernel data structure benchmark results.

Usage:
    python3 plot_bench.py

Reads bench_data.txt and produces bench_results.pdf with two
side-by-side plots: (a) Insert Performance, (b) Lookup Performance.
"""
import matplotlib.pyplot as plt

N = [100, 1000, 5000, 10000, 50000]

# TODO: Fill in your measurements (ns/op)
list_ins  = [0, 0, 0, 0, 0]
hash_ins  = [0, 0, 0, 0, 0]
rb_ins    = [0, 0, 0, 0, 0]
xa_ins    = [0, 0, 0, 0, 0]

list_lkp  = [0, 0, 0, 0, 0]
hash_lkp  = [0, 0, 0, 0, 0]
rb_lkp    = [0, 0, 0, 0, 0]
xa_lkp    = [0, 0, 0, 0, 0]

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(10, 4))

for ax, title, data in [
    (ax1, "(a) Insert Performance",
     [list_ins, hash_ins, rb_ins, xa_ins]),
    (ax2, "(b) Lookup Performance",
     [list_lkp, hash_lkp, rb_lkp, xa_lkp])]:
    labels = ["Linked List", "Hash Table", "RB-Tree", "XArray"]
    markers = ["o", "s", "^", "D"]
    for d, l, m in zip(data, labels, markers):
        ax.plot(N, d, marker=m, label=l)
    ax.set_xscale("log")
    ax.set_xlabel("Number of Entries (N)")
    ax.set_ylabel("Time per Operation (ns/op)")
    ax.set_title(title)
    ax.legend()
    ax.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig("bench_results.pdf")
plt.show()
