#include <algorithm>
#include <cstdint>
#include <forward_list>
#include <gtest/gtest.h>
#include <limits>
#include <list>
#include <memory>
#include <random>
#include <utility>
#include <vector>

#include "enranged/__detail/flat_list.hpp"
#include "enranged/sorting.hpp"

#include "linked_list.hpp"

using namespace enranged;

struct test_type {
  int value;
  size_t count;

  bool operator==(const test_type&) const noexcept = default;
};

template <typename T>
class SortingTests: public ::testing::Test {
public:
  constexpr static bool is_stability_test =
    std::same_as<typename T::value_type, test_type>;

protected:
  void build_test_vec(const size_t size) {
    test_vec = decltype(test_vec)(size);

    if constexpr (is_stability_test) {
      size_t ctr = 0;
      ranges::generate(this->test_vec, [&ctr]() {
        return test_type{rand() % 8, ++ctr};
      });
    }
    else {
      std::mt19937 gen{unsigned(rand())};
      ranges::generate(test_vec, [&gen]() {
        return std::uniform_int_distribution{}(gen);
      });
    }
  }

  void build_range() {
    range = T(test_vec.size());
    ranges::copy(test_vec, ranges::begin(range));
  }

  auto build_all_for_sorting(const size_t max_size) {
    const size_t skip_left = rand() % 10;
    const size_t skip_right = rand() % 10;

    const size_t size =
      1 + rand() % max_size + skip_left + skip_right;

    this->build_test_vec(size);
    this->build_range();

    return std::make_pair(skip_left, skip_right);
  }

  void test_sorted(const ranges::iterator_t<T> back_it,
                   const auto test_begin, const auto test_end) {
    if constexpr (is_stability_test)
      ranges::stable_sort(test_begin, test_end,
                          std::greater{}, &test_type::value);
    else
      ranges::sort(test_begin, test_end);

    EXPECT_EQ(*back_it, *ranges::prev(test_end));

    auto it = ranges::begin(range);
    for (auto val : test_vec) { ASSERT_EQ(*it++, val); }
  }

  T range;
  std::vector<typename T::value_type> test_vec;
};

using SpliceSortable =
  ::testing::Types<std::list<int>, std::forward_list<int>, linked_list<int>,
                   std::list<test_type>, std::forward_list<test_type>,
                   linked_list<test_type>>;
TYPED_TEST_SUITE(SortingTests, SpliceSortable);

TYPED_TEST(SortingTests, coinplace_merge_splice) {
  constexpr size_t EltsCount = 42;

  for (size_t left = 0; left < EltsCount; ++left) {
    for (size_t mid = left + 1; mid <= EltsCount; ++mid) {
      for (size_t right = mid; right <= EltsCount; ++right) {
        this->build_test_vec(EltsCount);

        const auto test_begin = ranges::next(this->test_vec.begin(), left);
        const auto test_mid = ranges::next(this->test_vec.begin(), mid);
        const auto test_end = ranges::next(test_mid, (right - mid));

        if constexpr (SortingTests<TypeParam>::is_stability_test) {
          ranges::stable_sort(test_begin, test_mid,
                              std::greater{}, &test_type::value);
          ranges::stable_sort(test_mid, test_end,
                              std::greater{}, &test_type::value);
        }
        else {
          ranges::sort(test_begin, test_mid);
          ranges::sort(test_mid, test_end);
        }

        this->build_range();

        const auto range_mid =
          ranges::next(ranges::begin(this->range), mid - 1);
        const auto range_last = ranges::next(range_mid, (right - mid));

        ranges::iterator_t<TypeParam> result;
        if (left == 0) {
          if constexpr (SortingTests<TypeParam>::is_stability_test)
            result = coinplace_merge_splice(this->range,
                                            before_begin(this->range),
                                            range_mid, range_last,
                                            std::greater{}, &test_type::value);
          else
            result = coinplace_merge_splice(this->range,
                                            before_begin(this->range),
                                            range_mid, range_last);
        }
        else {
          const auto range_left =
            ranges::next(ranges::begin(this->range), left - 1);
          if constexpr (SortingTests<TypeParam>::is_stability_test)
            result = coinplace_merge_splice(this->range, range_left,
                                            range_mid, range_last,
                                            std::greater{}, &test_type::value);
          else
            result = coinplace_merge_splice(this->range, range_left,
                                            range_mid, range_last);
        }

        this->test_sorted(result, test_begin, test_end);
      }
    }
  }
}

TYPED_TEST(SortingTests, insertion_sort_splice) {
  constexpr size_t Runs = 100;
  constexpr size_t MaxElts = 1000;

  for (size_t i = 0; i < Runs; ++i) {
    auto [skip_left, skip_right] =
      this->build_all_for_sorting(i == 0 ? 1 : MaxElts);

    ranges::iterator_t<TypeParam> result;
    if (skip_left == 0) {
      const auto count = this->test_vec.size() - skip_right;
      if constexpr (SortingTests<TypeParam>::is_stability_test)
        result =
          insertion_sort_splice(this->range, before_begin(this->range), count,
                                std::greater{}, &test_type::value);
      else
        result =
          insertion_sort_splice(this->range, before_begin(this->range), count);
    }
    else {
      const auto left = ranges::next(ranges::begin(this->range), skip_left - 1);
      const auto count = this->test_vec.size() - skip_left - skip_right;
      if constexpr (SortingTests<TypeParam>::is_stability_test)
        result = insertion_sort_splice(this->range, left, count,
                                       std::greater{}, &test_type::value);
      else
        result = insertion_sort_splice(this->range, left, count);
    }

    this->test_sorted(result, this->test_vec.begin() + skip_left,
                      this->test_vec.end() - skip_right);
  }
}

TYPED_TEST(SortingTests, merge_sort_splice) {
  constexpr size_t Runs = 100;
  constexpr size_t MaxElts = 1000;

  for (size_t i = 0; i < Runs; ++i) {
    auto [skip_left, skip_right] =
      this->build_all_for_sorting(i == 0 ? 1 : MaxElts);

    ranges::iterator_t<TypeParam> result;
    if (skip_left == 0) {
      const auto count = this->test_vec.size() - skip_right;
      if constexpr (SortingTests<TypeParam>::is_stability_test)
        result =
          merge_sort_splice(this->range, before_begin(this->range), count,
                            std::greater{}, &test_type::value);
      else
        result =
          merge_sort_splice(this->range, before_begin(this->range), count);
    }
    else {
      const auto left = ranges::next(ranges::begin(this->range), skip_left - 1);
      const auto count = this->test_vec.size() - skip_left - skip_right;
      if constexpr (SortingTests<TypeParam>::is_stability_test)
        result = merge_sort_splice(this->range, left, count,
                                   std::greater{}, &test_type::value);
      else
        result = merge_sort_splice(this->range, left, count);
    }

    this->test_sorted(result, this->test_vec.begin() + skip_left,
                      this->test_vec.end() - skip_right);
  }
}

TEST(FlatListTests, base) {
  // Test an internal structure used in sorting
  constexpr size_t MaxElts = 100;

  __detail::flat_list<size_t, MaxElts> list;
  std::forward_list<size_t> test_list;

  static_assert(ranges::forward_range<decltype(list)>);
  ASSERT_EQ(sizeof(decltype(list)::pos_t), 1);

  for (size_t i = 0; i < MaxElts; ++i) {
    EXPECT_EQ(list.size(), i);

    const auto des = rand() % 3;
    const auto pos = des == 0 ? 0 : (des == 1 ? i : (rand() % (i + 1)));
    const auto it =
      list.emplace_after(ranges::next(list.before_begin(), pos), i);

    EXPECT_EQ(*it, i);
    EXPECT_EQ(ranges::next(list.before_begin()), list.begin());
    EXPECT_EQ(ranges::next(list.begin(), list.size()), list.end());

    test_list.emplace_after(ranges::next(test_list.before_begin(), pos), i);

    size_t ctr = 0;
    auto test_it = test_list.begin();
    for (auto x : list) {
      ++ctr;
      ASSERT_EQ(x, *test_it++);
    }

    EXPECT_EQ(ctr, (i + 1));
  }
}

template <size_t _shift>
bool equal_shifts(const int x, const int y) noexcept {
  return x >> _shift == y >> _shift;
}

template <typename T, size_t _max_buckets = 1, size_t _shift = 0>
auto call_bs(T& range, const size_t skip_left,
             const size_t size, const size_t skip_right) {
  const auto invoker = [&range](const auto left, const auto right) {
    if constexpr (SortingTests<T>::is_stability_test)
      return bucket_sort_splice<_max_buckets>
        (range, left, right, equal_shifts<_shift>, &test_type::value,
         std::greater{}, &test_type::value);
    else
      return bucket_sort_splice<_max_buckets>(range, left, right,
                                              equal_shifts<_shift>);
  };

  if (skip_left == 0) {
    if (skip_right == 0)
      return invoker(before_begin(range), ranges::end(range));
    else
      return invoker(before_begin(range),
                     ranges::next(ranges::begin(range), skip_left + size));
  }
  else {
    const auto llim = ranges::next(ranges::begin(range), skip_left - 1);
    if (skip_right == 0)
      return invoker(llim, ranges::end(range));
    else
      return invoker(llim,
                     ranges::next(ranges::begin(range), skip_left + size));
  }
};

TYPED_TEST(SortingTests, bucket_sort_splice) {
  constexpr size_t Runs = 500;
  constexpr size_t MaxElts = 1000;

  std::vector<decltype(&call_bs<TypeParam>)> sorters;
  if constexpr (SortingTests<TypeParam>::is_stability_test)
    sorters = {
      &call_bs<TypeParam, 4, 1>, // exact match
      &call_bs<TypeParam, 2, 1>, // less buckets
      &call_bs<TypeParam, 8, 1>, // too many buckets
      &call_bs<TypeParam, 3, 3>, // all equiv
      &call_bs<TypeParam, 8, 0>  // all not equiv
    };
  else
    sorters =  {
      &call_bs<TypeParam, 32, 26>, // exact match
      &call_bs<TypeParam, 20, 25>, // less buckets
      &call_bs<TypeParam, 64, 27>, // too many buckets
      &call_bs<TypeParam, 16, 31>, // all equiv
      &call_bs<TypeParam, 8, 0>    // all not equiv
    };

  for (size_t i = 0; i < Runs; ++i) {
    auto [skip_left, skip_right] =
      this->build_all_for_sorting(i == 0 ? 1 : MaxElts);

    const auto size = this->test_vec.size() - skip_left - skip_right;
    const auto [out_size, last] =
      sorters[rand() % sorters.size()](this->range,
                                       skip_left, size, skip_right);

    EXPECT_EQ(out_size, size);
    this->test_sorted(last, this->test_vec.begin() + skip_left,
                      this->test_vec.end() - skip_right);
  }
}

class SortingListTests: public SortingTests<std::list<int>> {};

TEST_F(SortingListTests, alt_interfaces) {
  this->build_test_vec(100);
  const auto test_mid = ranges::next(this->test_vec.begin(), 42);

  ranges::sort(this->test_vec.begin(), test_mid);
  ranges::sort(test_mid, this->test_vec.end());

  this->build_range();
  const auto mid = ranges::next(ranges::begin(this->range), 41);
  const auto result = coinplace_merge_splice(this->range, mid);

  this->test_sorted(result, this->test_vec.begin(), this->test_vec.end());

  for (size_t i = 0; i < 2; ++i) {
    this->build_test_vec(100);
    this->build_range();

    const auto result = i == 0 ? insertion_sort_splice(this->range)
      : merge_sort_splice(this->range);

    this->test_sorted(result, this->test_vec.begin(), this->test_vec.end());
  }

  // Also test empty
  this->range.clear();
  const auto is_result = insertion_sort_splice(this->range);
  const auto ms_result = merge_sort_splice(this->range);

  ASSERT_EQ(is_result, ranges::end(this->range));
  ASSERT_EQ(ms_result, ranges::end(this->range));
}

TEST_F(SortingListTests, weakly_consistent_bucket_sort) {
  constexpr size_t Runs = 100;
  constexpr size_t MaxElts = 10000;

  // Test bucket sort with a weakly consistent but not (totally)
  // consistent with the order equality relation
  const auto comp = [](const int lhs, const int rhs) {
    return (lhs >> 27) < (rhs >> 27);
  };

  for (size_t i = 0; i < Runs; ++i) {
    this->build_test_vec(MaxElts);
    this->build_range();

    const auto [out_size, last] =
      bucket_sort_splice<32>(this->range, before_begin(this->range),
                             ranges::end(this->range),
                             equal_shifts<26>, {}, comp);

    EXPECT_EQ(out_size, this->test_vec.size());
    auto it = ranges::begin(this->range);
    for (size_t j = 1; j < out_size; ++j) {
      const auto next = ranges::next(it);
      ASSERT_LE((*it >> 27), (*next >> 27));
      it = next;
    }
  }
}

struct alloc_stats {
  size_t allocated = 0;
  size_t freed = 0;
};

template <typename T>
class counting_allocator {
public:
  using value_type = T;

  counting_allocator(alloc_stats& stats): stats_(stats) {}

  template <typename U>
  counting_allocator(counting_allocator<U>& rhs):
    stats_(rhs.stats_) {}

  T* allocate(const size_t size) {
    stats_.allocated+= sizeof(T) * size;
    return std::allocator<T>{}.allocate(size);
  }

  void deallocate(T* const ptr, const size_t size) {
    stats_.freed+= sizeof(T) * size;
    std::allocator<T>{}.deallocate(ptr, size);
  }

private:
  alloc_stats& stats_;
  template <typename U> friend class counting_allocator;
};

TEST_F(SortingListTests, bucket_sort_memory) {
  constexpr static size_t data_elt_size =
    sizeof(size_t) + sizeof(ranges::iterator_t<std::list<int>>);

  alloc_stats stats;
  const auto test_stats =
    [&stats](const size_t bucket_size, const size_t num_size) {
      EXPECT_GE(stats.allocated, bucket_size*(data_elt_size + num_size));
      EXPECT_LT(stats.allocated, (bucket_size + 1)*(data_elt_size + num_size));
      EXPECT_EQ(stats.freed, stats.allocated);
    };

  this->build_all_for_sorting(1000);

  counting_allocator<int> alloc(stats);

  bucket_sort_splice<32>(alloc, this->range, before_begin(this->range),
                         ranges::end(this->range), equal_shifts<26>);
  test_stats(32, 1);

  stats = alloc_stats{};
  counting_allocator<double> alloc2(stats);
  bucket_sort_splice<512>(alloc2, this->range, before_begin(this->range),
                          ranges::end(this->range), equal_shifts<11>);
  test_stats(512, 2);

  stats = alloc_stats{};
  try {
    bucket_sort_splice<65536>(alloc, this->range, before_begin(this->range),
                              ranges::end(this->range),
                              [](int, int) -> bool { throw 42; });
  }
  catch (int) {}
  test_stats(65536, 4);
}
