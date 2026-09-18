// Tests for knapsack::solve.
//
//  1. hand-checked textbook instances (CLRS 16.2-1 style, the README example, …);
//  2. structural properties every solution must satisfy (feasibility, at most one
//     fractional item, capacity exhausted unless everything fits, ordering);
//  3. optimality: on random small instances the greedy value is compared with a
//     brute-force LP over the vertices of the feasible region — for this problem
//     every vertex has at most one fractional coordinate, so enumerating
//     "subset taken whole + one item fractional" is exhaustive;
//  4. the relaxation dominates the 0/1 optimum (DP) and is tight when the greedy
//     solution happens to be integral.

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>

#include "knapsack/fractional.hpp"

namespace {

int failures = 0;
int checks = 0;

void expect(bool ok, const char* what) {
  ++checks;
  if (!ok) {
    ++failures;
    std::printf("  FAIL: %s\n", what);
  }
}

bool near(double a, double b, double eps = 1e-9) { return std::fabs(a - b) <= eps * std::max(1.0, std::fabs(b)); }

using knapsack::Item;

// --- 1. textbook instances -----------------------------------------------------

void hand_checked() {
  // CLRS §16.2: weights 10, 20, 30 with values 60, 100, 120, capacity 50.
  // Greedy takes items 1 and 2 whole and 2/3 of item 3: 60 + 100 + 80 = 240.
  {
    const auto s = knapsack::solve({{10, 60}, {20, 100}, {30, 120}}, 50);
    expect(near(s.total_value, 240.0), "CLRS instance value == 240");
    expect(near(s.fraction[0], 1.0) && near(s.fraction[1], 1.0) && near(s.fraction[2], 2.0 / 3.0), "CLRS fractions 1, 1, 2/3");
    expect(near(s.total_weight, 50.0), "CLRS uses the whole capacity");
  }
  // Original README example: one juice of 500 litres worth 30, stomach capacity 10.
  {
    const auto s = knapsack::solve({{500, 30}}, 10);
    expect(near(s.total_value, 0.6), "README example: 30 * 10/500 == 0.6");
    expect(near(s.fraction[0], 0.02), "README example: fraction 0.02");
  }
  // Everything fits: capacity is not exhausted, all fractions are 1.
  {
    const auto s = knapsack::solve({{1, 5}, {2, 3}}, 10);
    expect(near(s.total_value, 8.0) && near(s.total_weight, 3.0), "everything fits");
  }
  // Zero capacity and empty instance.
  expect(near(knapsack::max_value({{3, 7}}, 0), 0.0), "zero capacity");
  expect(near(knapsack::max_value({}, 42), 0.0), "no items");
  // Ties in density are broken by index (deterministic).
  {
    const auto s = knapsack::solve({{2, 4}, {1, 2}, {4, 8}}, 3);
    expect(s.order[0] == 0 && s.order[1] == 1 && s.order[2] == 2, "density ties keep input order");
    expect(near(s.total_value, 6.0), "tied densities: value 6");
  }
  // Densities compared without division: 1e-300-scale and 1e300-scale inputs.
  {
    const auto s = knapsack::solve({{1e300, 2e300}, {1e-300, 3e-300}}, 1e300);
    expect(s.order[0] == 1, "cross-product comparison handles extreme magnitudes");
  }
  // Validation.
  bool threw = false;
  try { (void)knapsack::solve({{0, 1}}, 5); } catch (const std::invalid_argument&) { threw = true; }
  expect(threw, "zero weight is rejected");
  threw = false;
  try { (void)knapsack::solve({{1, -1}}, 5); } catch (const std::invalid_argument&) { threw = true; }
  expect(threw, "negative value is rejected");
  threw = false;
  try { (void)knapsack::solve({{1, 1}}, -1); } catch (const std::invalid_argument&) { threw = true; }
  expect(threw, "negative capacity is rejected");
}

// --- 2/3. random instances vs. exhaustive vertex enumeration -------------------

// Best value over all vertices of {0 ≤ x ≤ 1, w·x ≤ C}: a subset S taken whole
// plus at most one item j ∉ S taken fractionally to fill the remaining capacity.
double brute_force(const std::vector<Item>& items, double capacity) {
  const std::size_t n = items.size();
  double best = 0.0;
  for (unsigned mask = 0; mask < (1u << n); ++mask) {
    double w = 0, v = 0;
    for (std::size_t i = 0; i < n; ++i)
      if (mask & (1u << i)) { w += items[i].weight; v += items[i].value; }
    if (w > capacity + 1e-12) continue;
    best = std::max(best, v);
    const double rem = capacity - w;
    for (std::size_t j = 0; j < n; ++j) {
      if (mask & (1u << j)) continue;
      const double x = std::min(1.0, rem / items[j].weight);
      best = std::max(best, v + x * items[j].value);
    }
  }
  return best;
}

void random_instances(int rounds) {
  std::mt19937 rng(20240917);
  std::uniform_int_distribution<int> n_dist(1, 10), w_dist(1, 20), v_dist(0, 50);
  std::uniform_real_distribution<double> cap_dist(0.0, 80.0);

  int integral_solutions = 0;
  for (int r = 0; r < rounds; ++r) {
    const auto n = static_cast<std::size_t>(n_dist(rng));
    std::vector<Item> items;
    for (std::size_t i = 0; i < n; ++i) items.push_back({double(w_dist(rng)), double(v_dist(rng))});
    const double capacity = (r % 2 == 0) ? std::floor(cap_dist(rng)) : cap_dist(rng);

    const auto s = knapsack::solve(items, capacity);

    // feasibility & structure
    double w = 0, v = 0;
    int fractional = 0;
    for (std::size_t i = 0; i < n; ++i) {
      expect(s.fraction[i] >= 0.0 && s.fraction[i] <= 1.0 + 1e-12, "fraction within [0,1]");
      w += s.fraction[i] * items[i].weight;
      v += s.fraction[i] * items[i].value;
      if (s.fraction[i] > 1e-12 && s.fraction[i] < 1.0 - 1e-12) ++fractional;
    }
    expect(w <= capacity + 1e-9, "solution is feasible");
    expect(near(v, s.total_value), "reported value matches the fractions");
    expect(near(w, s.total_weight), "reported weight matches the fractions");
    expect(fractional <= 1, "at most one item is fractional");
    double total_w = 0;
    for (const auto& it : items) total_w += it.weight;
    if (total_w > capacity) expect(near(w, capacity), "capacity exhausted when items do not all fit");
    for (std::size_t k = 1; k < s.order.size(); ++k) {
      const Item &a = items[s.order[k - 1]], &b = items[s.order[k]];
      expect(a.value * b.weight >= b.value * a.weight - 1e-9, "order is non-increasing in density");
    }

    // optimality vs. exhaustive enumeration
    expect(near(s.total_value, brute_force(items, capacity)), "greedy value equals exhaustive LP optimum");

    // relaxation dominates the 0/1 optimum; equal whenever the greedy solution is integral
    if (r % 2 == 0) {
      const double dp = knapsack::zero_one_dp(items, static_cast<long long>(capacity));
      expect(s.total_value >= dp - 1e-9, "fractional optimum >= 0/1 optimum");
      if (fractional == 0) { expect(near(s.total_value, dp), "integral greedy solution is 0/1-optimal"); ++integral_solutions; }
    }
  }
  expect(integral_solutions > 0, "random suite exercised the integral case");
}

}  // namespace

int main(int argc, char** argv) {
  const int rounds = argc > 1 ? std::atoi(argv[1]) : 20000;
  hand_checked();
  random_instances(rounds);
  std::printf("%d checks, %d failures\n", checks, failures);
  std::puts(failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED");
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
