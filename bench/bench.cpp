// bench — two experiments that back the README:
//
//   1. running time of the greedy O(n log n) fractional solver vs. the O(n·W)
//      0/1 dynamic program, for growing n (weights in [1, 1000], W = 25 % of Σw);
//   2. the "integrality gap": how far the fractional optimum (an upper bound used
//      by branch-and-bound) sits above the true 0/1 optimum on random instances.
//
// Output is CSV on stdout; bench/plot.py turns it into docs/figures/bench.png.

#include <chrono>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <string>
#include <random>
#include <vector>

#include "knapsack/fractional.hpp"

namespace {

using Clock = std::chrono::steady_clock;

std::vector<knapsack::Item> random_items(std::mt19937& rng, std::size_t n, int max_w, int max_v) {
  std::uniform_int_distribution<int> w(1, max_w), v(1, max_v);
  std::vector<knapsack::Item> items;
  items.reserve(n);
  for (std::size_t i = 0; i < n; ++i) items.push_back({double(w(rng)), double(v(rng))});
  return items;
}

double capacity_quarter(const std::vector<knapsack::Item>& items) {
  double total = 0;
  for (const auto& it : items) total += it.weight;
  return std::floor(total / 4.0);
}

template <class F>
double time_ms(F&& f, int repeats) {
  const auto t0 = Clock::now();
  for (int i = 0; i < repeats; ++i) f();
  return std::chrono::duration<double, std::milli>(Clock::now() - t0).count() / repeats;
}

}  // namespace

int main(int argc, char** argv) {
  std::mt19937 rng(7);
  const bool gap_only = argc > 1 && std::string(argv[1]) == "--gap";
  const bool time_only = argc > 1 && std::string(argv[1]) == "--time";

  if (!gap_only) {
    std::puts("experiment,n,capacity,greedy_ms,dp_ms");
    for (const std::size_t n : {100u, 200u, 500u, 1000u, 2000u, 5000u, 10000u, 20000u}) {
      const auto items = random_items(rng, n, 1000, 1000);
      const double W = capacity_quarter(items);
      volatile double sink = 0;
      const int reps = n <= 1000 ? 20 : 5;
      const double g = time_ms([&] { sink = knapsack::max_value(items, W); }, reps);
      const double d = time_ms([&] { sink = knapsack::zero_one_dp(items, static_cast<long long>(W)); }, n <= 2000 ? 3 : 1);
      std::printf("time,%zu,%.0f,%.4f,%.3f\n", n, W, g, d);
      std::fflush(stdout);
    }
  }
  if (!time_only) {
    if (!gap_only) std::puts("");
    std::puts("experiment,n,instances,mean_gap_pct,max_gap_pct,greedy_integral_pct");
    for (const std::size_t n : {5u, 10u, 20u, 50u, 100u, 200u}) {
      const int instances = 300;
      double sum_gap = 0, max_gap = 0;
      int integral = 0;
      for (int k = 0; k < instances; ++k) {
        const auto items = random_items(rng, n, 100, 100);
        const double W = capacity_quarter(items);
        const auto s = knapsack::solve(items, W);
        const double dp = knapsack::zero_one_dp(items, static_cast<long long>(W));
        const double gap = dp > 0 ? 100.0 * (s.total_value - dp) / dp : 0.0;
        sum_gap += gap;
        if (gap > max_gap) max_gap = gap;
        bool is_integral = true;
        for (double x : s.fraction) if (x > 1e-12 && x < 1 - 1e-12) is_integral = false;
        if (is_integral) ++integral;
      }
      std::printf("gap,%zu,%d,%.4f,%.4f,%.1f\n", n, instances, sum_gap / instances, max_gap, 100.0 * integral / instances);
    }
  }
  return 0;
}
