# 🧃 Juice Happiness — Fractional Knapsack Problem (Greedy Algorithm)

[![Language: C++](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg?style=for-the-badge&logo=c%2B%2B)](https://isocpp.org/)
[![Topic: Algorithms](https://img.shields.io/badge/Algorithm-Greedy_Approach-orange.svg?style=for-the-badge)]()
[![Domain: Optimization](https://img.shields.io/badge/Problem-Fractional_Knapsack-green.svg?style=for-the-badge)]()

An optimal C++ solution for the **Fractional Knapsack Problem** using a **Greedy Strategy**, designed to maximize cumulative happiness under capacity constraints.

---

## 📌 Problem Description

Amin visits a juice shop with a limited stomach capacity of $V$ liters. The shop offers $N$ distinct juice types, where the $i$-th juice container has:
- A total volume of $v_i$ liters.
- A total happiness gain of $h_i$ if completely consumed.

Since fractional consumption is allowed, drinking any fraction $\alpha \in [0, 1]$ yields $\alpha \cdot h_i$ happiness. The objective is to determine the non-negative volume taken from each juice type to maximize total happiness without exceeding capacity $V$.

---

## 💡 Algorithmic Approach

This problem is solved using the **Greedy Choice Property**:
1. **Density Calculation:** Compute the happiness density ratio for each juice:
   $$\text{ratio}_i = \frac{h_i}{v_i}$$
2. **Sorting:** Sort all available juices in descending order of their density ratio: $O(N \log N)$.
3. **Greedy Selection:**
   - Consume full containers as long as remaining stomach capacity accommodates them.
   - For the final remaining capacity, take a fraction $\frac{V_{\text{remaining}}}{v_i}$ of the current best juice and terminate.

### Complexity Analysis
- **Time Complexity:** $O(N \log N)$ dominated by sorting.
- **Space Complexity:** $O(N)$ for storing item properties.

---

## 📥 Sample I/O

### Input
```text
1 10
500 30
