#include <benchmark/benchmark.h>
#include <cstdlib>
#include <forward_list>
#include <list>
#include <random>
#include <ranges>
#include <vector>

#include "enranged/sorting.hpp"

namespace ranges = std::ranges;

constexpr size_t MinSize = 10;
constexpr size_t MaxSize = 10000000ull;
constexpr size_t Multiplier = 10;

// We use a single vector of random data for every benchmark
const static auto test_vec = []() {
  std::vector<int> result(MaxSize);

  std::random_device rd;
  std::mt19937 gen{rd()};
  ranges::generate(result, [&gen]() {
    return std::uniform_int_distribution{}(gen);
  });

  return result;
 }();

template <typename T>
class SortingBenchmarks: public benchmark::Fixture {
protected:
  void rebuild_stl_list(::benchmark::State& state) {
    state.PauseTiming();  // This has some performance penalty but it
                          // doesn't matter with our orders
    range.clear();
    range = T(test_vec.begin(), test_vec.begin() + state.range(0));

    benchmark::ClobberMemory();
    state.ResumeTiming();
  }

  T range;
};

BENCHMARK_TEMPLATE_DEFINE_F(SortingBenchmarks, merge_sort_list,
                            std::list<int>)(benchmark::State& state) {
  for (auto _ : state) {
    this->rebuild_stl_list(state);
    enranged::merge_sort_splice(this->range,
                                enranged::before_begin(this->range),
                                ranges::size(this->range));
  }
}

BENCHMARK_TEMPLATE_DEFINE_F(SortingBenchmarks, merge_sort_forward_list,
                            std::forward_list<int>)(benchmark::State& state) {
  for (auto _ : state) {
    this->rebuild_stl_list(state);
    enranged::merge_sort_splice(this->range,
                                enranged::before_begin(this->range),
                                state.range(0));
  }
}

BENCHMARK_REGISTER_F(SortingBenchmarks, merge_sort_list)
  ->RangeMultiplier(Multiplier)->Range(MinSize, MaxSize)
  ->Unit(benchmark::kMicrosecond);

BENCHMARK_REGISTER_F(SortingBenchmarks, merge_sort_forward_list)
  ->RangeMultiplier(Multiplier)->Range(MinSize, MaxSize)
  ->Unit(benchmark::kMicrosecond);
