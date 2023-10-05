#include <benchmark/benchmark.h>
#include <cstdlib>
#include <forward_list>
#include <list>
#include <memory>
#include <random>
#include <ranges>
#include <vector>

#include "enranged/sorting.hpp"

#include "linked_list.hpp"  // from test
#include "shuffled_memory_resource.hpp"

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

static shuffled_memory_resource<MaxSize + 42> memory_resource{};

template <typename T>
class shuffled_allocator {
public:
  static_assert(sizeof(T) <= decltype(memory_resource)::max_alloc);
  using value_type = T;

  shuffled_allocator() noexcept = default;

  template <typename U>
  shuffled_allocator(const shuffled_allocator<U>&) noexcept {}

  bool operator==(const shuffled_allocator&) const noexcept = default;

  static size_t max_size() noexcept {
    return 1;
  }

  T* allocate(const size_t size) const noexcept {
    if (size != 1) return nullptr;
    return static_cast<T*>(memory_resource.allocate());
  }

  void deallocate(void*, const size_t) const noexcept {}
};

template <typename T>
class SortingBenchmarks: public benchmark::Fixture {
protected:
  T& rebuild_list(::benchmark::State& state) {
    state.PauseTiming();  // This has some performance penalty but it
                          // doesn't matter with our orders
    range_.reset();
    memory_resource.reset();

    range_ = std::make_unique<T>(test_vec.begin(),
                                 test_vec.begin() + state.range(0));

    benchmark::ClobberMemory();
    state.ResumeTiming();

    return *range_;
  }

  void TearDown(const ::benchmark::State& state) {
    range_.reset();
  }

private:
  std::unique_ptr<T> range_;
};

bool eq_rel(const int x, const int y) noexcept {
  return x >> 26 == y >> 26;
}

BENCHMARK_TEMPLATE_DEFINE_F(SortingBenchmarks, merge_sort_list,
                            std::list<int, shuffled_allocator<int>>)
  (benchmark::State& state) {
  for (auto _ : state) {
    auto& range = this->rebuild_list(state);
    enranged::merge_sort_splice(range,
                                enranged::before_begin(range),
                                ranges::size(range));
  }
}

BENCHMARK_TEMPLATE_DEFINE_F(SortingBenchmarks, bucket_sort_list,
                            std::list<int, shuffled_allocator<int>>)
  (benchmark::State& state) {
  for (auto _ : state) {
    auto& range = this->rebuild_list(state);
    enranged::bucket_sort_splice(range,
                                 enranged::before_begin(range),
                                 ranges::end(range), eq_rel);
  }
}

BENCHMARK_TEMPLATE_DEFINE_F(SortingBenchmarks, std_sort_list,
                            std::list<int, shuffled_allocator<int>>)
  (benchmark::State& state) {
  for (auto _ : state) {
    auto& range = this->rebuild_list(state);
    range.sort();
  }
}

BENCHMARK_TEMPLATE_DEFINE_F(SortingBenchmarks, merge_sort_forward_list,
                            std::forward_list<int, shuffled_allocator<int>>)
  (benchmark::State& state) {
  for (auto _ : state) {
    auto& range = this->rebuild_list(state);
    enranged::merge_sort_splice(range,
                                enranged::before_begin(range),
                                state.range(0));
  }
}

BENCHMARK_TEMPLATE_DEFINE_F(SortingBenchmarks, bucket_sort_forward_list,
                            std::forward_list<int, shuffled_allocator<int>>)
  (benchmark::State& state) {
  for (auto _ : state) {
    auto& range = this->rebuild_list(state);
    enranged::bucket_sort_splice(range,
                                 enranged::before_begin(range),
                                 ranges::end(range), eq_rel);
  }
}

BENCHMARK_TEMPLATE_DEFINE_F(SortingBenchmarks, std_sort_forward_list,
                            std::forward_list<int, shuffled_allocator<int>>)
  (benchmark::State& state) {
  for (auto _ : state) {
    auto& range = this->rebuild_list(state);
    range.sort();
  }
}

BENCHMARK_TEMPLATE_DEFINE_F(SortingBenchmarks, merge_sort_linked_list,
                            linked_list<int, shuffled_allocator<int>>)
  (benchmark::State& state) {
  for (auto _ : state) {
    auto& range = this->rebuild_list(state);
    enranged::merge_sort_splice(range,
                                enranged::before_begin(range),
                                ranges::size(range));
  }
}

BENCHMARK_TEMPLATE_DEFINE_F(SortingBenchmarks, bucket_sort_linked_list,
                            linked_list<int, shuffled_allocator<int>>)
  (benchmark::State& state) {
  for (auto _ : state) {
    auto& range = this->rebuild_list(state);
    enranged::bucket_sort_splice<32>(range,
                                     enranged::before_begin(range),
                                     ranges::end(range), eq_rel);
  }
}

BENCHMARK_REGISTER_F(SortingBenchmarks, merge_sort_list)
  ->RangeMultiplier(Multiplier)->Range(MinSize, MaxSize)
  ->Unit(benchmark::kMicrosecond);

BENCHMARK_REGISTER_F(SortingBenchmarks, bucket_sort_list)
  ->RangeMultiplier(Multiplier)->Range(MinSize, MaxSize)
  ->Unit(benchmark::kMicrosecond);

BENCHMARK_REGISTER_F(SortingBenchmarks, std_sort_list)
  ->RangeMultiplier(Multiplier)->Range(MinSize, MaxSize)
  ->Unit(benchmark::kMicrosecond);

BENCHMARK_REGISTER_F(SortingBenchmarks, merge_sort_forward_list)
  ->RangeMultiplier(Multiplier)->Range(MinSize, MaxSize)
  ->Unit(benchmark::kMicrosecond);

BENCHMARK_REGISTER_F(SortingBenchmarks, bucket_sort_forward_list)
  ->RangeMultiplier(Multiplier)->Range(MinSize, MaxSize)
  ->Unit(benchmark::kMicrosecond);

BENCHMARK_REGISTER_F(SortingBenchmarks, std_sort_forward_list)
  ->RangeMultiplier(Multiplier)->Range(MinSize, MaxSize)
  ->Unit(benchmark::kMicrosecond);

BENCHMARK_REGISTER_F(SortingBenchmarks, merge_sort_linked_list)
  ->RangeMultiplier(Multiplier)->Range(MinSize, MaxSize)
  ->Unit(benchmark::kMicrosecond);

BENCHMARK_REGISTER_F(SortingBenchmarks, bucket_sort_linked_list)
  ->RangeMultiplier(Multiplier)->Range(MinSize, MaxSize)
  ->Unit(benchmark::kMicrosecond);
