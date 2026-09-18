// fractional.hpp — the fractional (continuous) knapsack problem, solved greedily.
//
//   maximise   Σ x_i · value_i
//   subject to Σ x_i · weight_i ≤ capacity,   0 ≤ x_i ≤ 1
//
// Items are taken in decreasing order of value density value_i / weight_i; the
// first item that does not fit is taken fractionally and the algorithm stops.
// This greedy choice is optimal for the fractional relaxation (see README for
// the exchange-argument proof) and runs in O(n log n) time, O(n) extra space.
//
// Header-only, C++17, no dependencies.  SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

namespace knapsack {

/// One item: a (weight, value) pair.  Weights must be positive; values non-negative.
struct Item {
  double weight;
  double value;
};

/// The greedy solution.
struct Solution {
  double total_value = 0.0;      ///< optimal objective value
  double total_weight = 0.0;     ///< weight actually used (== capacity unless everything fits)
  std::vector<double> fraction;  ///< x_i ∈ [0, 1] for every item, in the ORIGINAL order
  std::vector<std::size_t> order;  ///< item indices sorted by decreasing density (ties by index)
};

/// Validates the instance; throws std::invalid_argument on a malformed input.
inline void validate(const std::vector<Item>& items, double capacity) {
  if (!(capacity >= 0.0)) throw std::invalid_argument("capacity must be a non-negative number");
  for (std::size_t i = 0; i < items.size(); ++i) {
    if (!(items[i].weight > 0.0)) throw std::invalid_argument("item " + std::to_string(i) + ": weight must be > 0");
    if (!(items[i].value >= 0.0)) throw std::invalid_argument("item " + std::to_string(i) + ": value must be >= 0");
  }
}

/// Item indices sorted by decreasing value density; ties broken by index so the
/// result is deterministic.  Densities are compared as cross products
/// (v_a · w_b > v_b · w_a) to avoid the division and its rounding.
inline std::vector<std::size_t> density_order(const std::vector<Item>& items) {
  std::vector<std::size_t> order(items.size());
  std::iota(order.begin(), order.end(), std::size_t{0});
  std::stable_sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
    return items[a].value * items[b].weight > items[b].value * items[a].weight;
  });
  return order;
}

/// Solve the fractional knapsack instance.  O(n log n).
inline Solution solve(const std::vector<Item>& items, double capacity) {
  validate(items, capacity);

  Solution s;
  s.fraction.assign(items.size(), 0.0);
  s.order = density_order(items);

  double remaining = capacity;
  for (const std::size_t i : s.order) {
    if (remaining <= 0.0) break;
    const Item& it = items[i];
    if (it.weight <= remaining) {  // whole item fits
      s.fraction[i] = 1.0;
      s.total_value += it.value;
      s.total_weight += it.weight;
      remaining -= it.weight;
    } else {  // take the fraction that fills the knapsack exactly, then stop
      const double x = remaining / it.weight;
      s.fraction[i] = x;
      s.total_value += x * it.value;
      s.total_weight += remaining;
      remaining = 0.0;
      break;
    }
  }
  return s;
}

/// Convenience overload returning only the optimal value.
inline double max_value(const std::vector<Item>& items, double capacity) {
  return solve(items, capacity).total_value;
}

// ---------------------------------------------------------------------------
// Reference solvers used by the tests and the README comparison.
// ---------------------------------------------------------------------------

/// 0/1 knapsack (items are indivisible) by dynamic programming over integer
/// weights; O(n · W) time, O(W) space.  Weights and capacity must be integers.
inline double zero_one_dp(const std::vector<Item>& items, long long capacity) {
  if (capacity < 0) return 0.0;
  std::vector<double> best(static_cast<std::size_t>(capacity) + 1, 0.0);
  for (const Item& it : items) {
    const long long w = static_cast<long long>(it.weight);
    if (w <= 0 || w > capacity) continue;
    for (long long c = capacity; c >= w; --c) {
      const auto ci = static_cast<std::size_t>(c), cw = static_cast<std::size_t>(c - w);
      best[ci] = std::max(best[ci], best[cw] + it.value);
    }
  }
  return best.back();
}

/// Upper bound used by branch-and-bound solvers for 0/1 knapsack: the value of
/// the fractional relaxation is never smaller than the 0/1 optimum.
inline double fractional_upper_bound(const std::vector<Item>& items, double capacity) {
  return max_value(items, capacity);
}

}  // namespace knapsack
