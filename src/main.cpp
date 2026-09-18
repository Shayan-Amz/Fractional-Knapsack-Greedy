// knapsack — command-line solver for the fractional knapsack problem.
//
//   knapsack [FILE]              read an instance from FILE (or stdin), print the plan
//   knapsack --interactive       prompt for the input like the original course program
//   knapsack --json [FILE]       machine-readable output
//
// Instance format (whitespace separated):
//
//   n capacity
//   weight_1 value_1
//   ...
//   weight_n value_n
//
// The original assignment phrased the problem as "juices": each of n juices has
// a volume (weight) and a happiness (value); a stomach of `capacity` litres is to
// be filled with the happiest possible mix, and any fraction of a juice may be drunk.

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "knapsack/fractional.hpp"

namespace {

void usage() {
  std::puts(
      "usage: knapsack [--json] [FILE]        solve the instance in FILE (default: stdin)\n"
      "       knapsack --interactive          prompt for the input\n"
      "\n"
      "instance format:  n capacity\\n  then n lines of  weight value\n"
      "                  (weights > 0, values >= 0; fractions of an item may be taken)");
}

struct Instance {
  double capacity = 0;
  std::vector<knapsack::Item> items;
};

Instance read_instance(std::istream& in) {
  Instance inst;
  std::size_t n = 0;
  if (!(in >> n >> inst.capacity)) throw std::runtime_error("expected: n capacity");
  inst.items.reserve(n);
  for (std::size_t i = 0; i < n; ++i) {
    knapsack::Item it{};
    if (!(in >> it.weight >> it.value)) throw std::runtime_error("expected " + std::to_string(n) + " lines of: weight value");
    inst.items.push_back(it);
  }
  return inst;
}

Instance read_interactive() {
  Instance inst;
  std::size_t n = 0;
  std::cout << "Capacity (e.g. stomach volume in litres): ";
  if (!(std::cin >> inst.capacity)) throw std::runtime_error("invalid capacity");
  std::cout << "Number of items (juices): ";
  if (!(std::cin >> n)) throw std::runtime_error("invalid item count");
  for (std::size_t i = 0; i < n; ++i) {
    knapsack::Item it{};
    std::cout << "Item " << (i + 1) << " — weight (volume) and value (happiness): ";
    if (!(std::cin >> it.weight >> it.value)) throw std::runtime_error("invalid item");
    inst.items.push_back(it);
  }
  return inst;
}

void print_plan(const Instance& inst, const knapsack::Solution& s) {
  std::printf("%-6s %10s %10s %10s %10s %10s\n", "item", "weight", "value", "density", "fraction", "taken");
  for (const std::size_t i : s.order) {
    const auto& it = inst.items[i];
    std::printf("%-6zu %10.4g %10.4g %10.4f %10.4f %10.4g\n", i + 1, it.weight, it.value, it.value / it.weight, s.fraction[i],
                s.fraction[i] * it.weight);
  }
  std::printf("\ncapacity used : %.6g of %.6g\n", s.total_weight, inst.capacity);
  std::printf("maximum value : %.6g\n", s.total_value);
}

void print_json(const Instance& inst, const knapsack::Solution& s) {
  std::printf("{\n  \"capacity\": %.17g,\n  \"total_value\": %.17g,\n  \"total_weight\": %.17g,\n  \"items\": [\n", inst.capacity,
              s.total_value, s.total_weight);
  for (std::size_t i = 0; i < inst.items.size(); ++i) {
    std::printf("    {\"index\": %zu, \"weight\": %.17g, \"value\": %.17g, \"fraction\": %.17g}%s\n", i + 1, inst.items[i].weight,
                inst.items[i].value, s.fraction[i], i + 1 < inst.items.size() ? "," : "");
  }
  std::puts("  ]\n}");
}

}  // namespace

int main(int argc, char** argv) {
  bool json = false, interactive = false;
  std::string file;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "-h" || arg == "--help") { usage(); return 0; }
    if (arg == "--json") json = true;
    else if (arg == "--interactive" || arg == "-i") interactive = true;
    else if (!arg.empty() && arg[0] == '-' && arg != "-") { std::fprintf(stderr, "unknown option: %s\n", arg.c_str()); usage(); return 2; }
    else file = arg;
  }

  try {
    Instance inst;
    if (interactive) {
      inst = read_interactive();
    } else if (!file.empty() && file != "-") {
      std::ifstream in(file);
      if (!in) throw std::runtime_error("cannot open " + file);
      inst = read_instance(in);
    } else {
      inst = read_instance(std::cin);
    }

    const knapsack::Solution s = knapsack::solve(inst.items, inst.capacity);
    if (json) print_json(inst, s);
    else {
      if (interactive) std::puts("");
      print_plan(inst, s);
    }
    return 0;
  } catch (const std::exception& e) {
    std::fprintf(stderr, "error: %s\n", e.what());
    return 1;
  }
}
