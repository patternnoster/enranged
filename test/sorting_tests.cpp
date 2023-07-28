#include <algorithm>
#include <forward_list>
#include <gtest/gtest.h>
#include <limits>
#include <list>
#include <random>
#include <utility>
#include <vector>

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
protected:
  constexpr static bool is_stability_test =
    std::same_as<typename T::value_type, test_type>;

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
