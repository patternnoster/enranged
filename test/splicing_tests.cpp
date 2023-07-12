#include <forward_list>
#include <gtest/gtest.h>
#include <list>
#include <tuple>

#include "enranged/splicing.hpp"

using namespace enranged;

template <typename T>
class SplicingTests: public ::testing::Test {
private:
  using test_range_t = std::forward_list<typename T::value_type>;

  template <typename Cont>
  auto build_range(const size_t size) {
    Cont result(size);
    ranges::copy(ranges::iota_view{size_t(0), size},
                 ranges::begin(result));
    return result;
  }

  std::tuple<T&, T&, test_range_t&, test_range_t&> rebuild_ranges
    (const size_t size, const bool same_ranges) {
    range1_ = build_range<T>(size);
    test_range1_ = build_range<test_range_t>(size);

    if (same_ranges)
      return { range1_, range1_, test_range1_, test_range1_ };

    range2_ = build_range<T>(size);
    test_range2_ = build_range<test_range_t>(size);
    return { range1_, range2_, test_range1_, test_range2_ };
  }

  auto test_equal_ranges(const T& result,
                         const test_range_t& expected) noexcept {
    auto it = ranges::begin(result);
    for (const auto& val : expected) { EXPECT_EQ(*it++, val); }
    EXPECT_EQ(it, ranges::end(result));
  }

protected:
  void test_cosplice_single(const size_t size, const bool same_ranges) {
    for (size_t pos = 0; pos <= size; ++pos) {
      for (size_t elt = 0; elt < size; ++elt) {
        if (same_ranges && (pos == elt || pos == elt + 1))
          continue;  // Skip UB

        auto [range1, range2, test_range1, test_range2]
          = rebuild_ranges(size, same_ranges);

        // Do the main splicing
        if (pos == 0) {
          if (elt == 0)
            cosplice(range1, before_begin(range1),
                     range2, before_begin(range2));
          else
            cosplice(range1, before_begin(range1), range2,
                     ranges::next(ranges::begin(range2), elt - 1));
        } else {
          const auto pos_it = ranges::next(ranges::begin(range1), pos - 1);

          if (elt == 0)
            cosplice(range1, pos_it, range2, before_begin(range2));
          else
            cosplice(range1, pos_it,
                     range2, ranges::next(ranges::begin(range2), elt - 1));
        }

        // Do the equivalent thing with a test range using STL
        test_range1.splice_after(ranges::next(test_range1.before_begin(), pos),
                                 test_range2,
                                 ranges::next(test_range2.before_begin(), elt));

        // And finally compare
        test_equal_ranges(range1, test_range1);
        if (!same_ranges) test_equal_ranges(range2, test_range2);
      }
    }
  }

  void test_cosplice_range(const size_t size, const bool same_ranges) {
    for (size_t pos = 0; pos <= size; ++pos) {
      for (size_t left = 0; left < size; ++left) {
        for (size_t right = left + 1; right <= size; ++right) {
          if (same_ranges && pos >= left && pos <= right)
            continue;  // Skip Ub

          auto [range1, range2, test_range1, test_range2]
            = rebuild_ranges(size, same_ranges);

          // Do the main splicing
          auto right_it = ranges::next(ranges::begin(range2), right - 1);
          if (pos == 0) {
            if (left == 0)
              cosplice(range1, before_begin(range1),
                       range2, before_begin(range2), right_it);
            else
              cosplice(range1, before_begin(range1),
                       range2, ranges::next(ranges::begin(range2), left - 1),
                       right_it);
          }
          else {
            const auto pos_it = ranges::next(ranges::begin(range1), pos - 1);

            if (left == 0)
              cosplice(range1, pos_it,
                       range2, enranged::before_begin(range2), right_it);
            else
              cosplice(range1, pos_it,
                       range2, ranges::next(ranges::begin(range2), left - 1),
                       right_it);
          }

          // Do the equivalent thing with a test range using STL
          test_range1.splice_after(ranges::next(test_range1.before_begin(),
                                                pos),
                                   test_range2,
                                   ranges::next(test_range2.before_begin(),
                                                left),
                                   ranges::next(test_range2.begin(), right));

          // And finally compare
          test_equal_ranges(range1, test_range1);
          if (!same_ranges) test_equal_ranges(range2, test_range2);
        }
      }
    }
  }

private:
  T range1_, range2_;
  test_range_t test_range1_, test_range2_;
};

using Spliceable = ::testing::Types<std::list<int>, std::forward_list<int>>;
TYPED_TEST_SUITE(SplicingTests, Spliceable);

TYPED_TEST(SplicingTests, concepts) {
  EXPECT_TRUE((spliceable_range<TypeParam>));
  EXPECT_TRUE((spliceable_with_range<TypeParam, TypeParam>));
}

TYPED_TEST(SplicingTests, cosplice_inplace_single) {
  constexpr size_t EltsCount = 10;
  this->test_cosplice_single(EltsCount, /*same_ranges=*/true);
}

TYPED_TEST(SplicingTests, cosplice_inplace_range) {
  constexpr size_t EltsCount = 10;
  this->test_cosplice_range(EltsCount, /*same_ranges=*/true);
}

TYPED_TEST(SplicingTests, cosplice_single) {
  constexpr size_t EltsCount = 10;
  this->test_cosplice_single(EltsCount, /*same_ranges=*/false);
}

TYPED_TEST(SplicingTests, cosplice_range) {
  constexpr size_t EltsCount = 10;
  this->test_cosplice_range(EltsCount, /*same_ranges=*/false);
}
