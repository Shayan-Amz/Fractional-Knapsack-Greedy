cmake_minimum_required(VERSION 3.14)
project(fractional_knapsack VERSION 2.0.0 LANGUAGES CXX
        DESCRIPTION "Greedy solver for the fractional knapsack problem, with proof, tests and benchmarks")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(KNAPSACK_BUILD_TESTS "Build the test suite" ON)
option(KNAPSACK_BUILD_BENCH "Build the benchmark" ON)

set(KNAPSACK_WARNINGS
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->)

# ---- header-only library ---------------------------------------------------
add_library(knapsack INTERFACE)
add_library(knapsack::knapsack ALIAS knapsack)
target_include_directories(knapsack INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(knapsack INTERFACE cxx_std_17)

# ---- command-line solver ---------------------------------------------------
add_executable(knapsack_cli src/main.cpp)
set_target_properties(knapsack_cli PROPERTIES OUTPUT_NAME knapsack)
target_link_libraries(knapsack_cli PRIVATE knapsack)
target_compile_options(knapsack_cli PRIVATE ${KNAPSACK_WARNINGS})

# ---- tests -----------------------------------------------------------------
if(KNAPSACK_BUILD_TESTS)
  enable_testing()
  add_executable(test_fractional tests/test_fractional.cpp)
  target_link_libraries(test_fractional PRIVATE knapsack)
  target_compile_options(test_fractional PRIVATE ${KNAPSACK_WARNINGS})
  add_test(NAME unit_and_random COMMAND test_fractional 20000)
  add_test(NAME cli_clrs COMMAND knapsack_cli ${CMAKE_CURRENT_SOURCE_DIR}/examples/clrs.txt)
  set_tests_properties(cli_clrs PROPERTIES PASS_REGULAR_EXPRESSION "maximum value : 240")
endif()

# ---- benchmark -------------------------------------------------------------
if(KNAPSACK_BUILD_BENCH)
  add_executable(bench bench/bench.cpp)
  target_link_libraries(bench PRIVATE knapsack)
  target_compile_options(bench PRIVATE ${KNAPSACK_WARNINGS})
endif()

install(DIRECTORY include/ DESTINATION include)
install(TARGETS knapsack_cli RUNTIME DESTINATION bin)
