# Minimal Makefile for environments without CMake.
#   make          build the solver and the tests
#   make test     run the tests
#   make bench    run the benchmark (prints CSV)
#   make clean

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
INCLUDES  = -Iinclude
BUILD     = build

.PHONY: all test bench clean

all: $(BUILD)/knapsack $(BUILD)/test_fractional

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/knapsack: src/main.cpp include/knapsack/fractional.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

$(BUILD)/test_fractional: tests/test_fractional.cpp include/knapsack/fractional.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

$(BUILD)/bench: bench/bench.cpp include/knapsack/fractional.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

test: $(BUILD)/test_fractional $(BUILD)/knapsack
	./$(BUILD)/test_fractional 20000
	./$(BUILD)/knapsack examples/clrs.txt

bench: $(BUILD)/bench
	./$(BUILD)/bench

clean:
	rm -rf $(BUILD)
