#include <algorithm>
#include <forward_list>
#include <gtest/gtest.h>
#include <limits>
#include <list>
#include <random>
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