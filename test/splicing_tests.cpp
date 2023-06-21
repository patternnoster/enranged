#include <forward_list>
#include <gtest/gtest.h>
#include <list>

#include "enranged/limits.hpp"
#include "enranged/splicing.hpp"

using namespace enranged;

template <typename T>
class SplicingTests: public ::testing::Test {};

using Spliceable = ::testing::Types<std::list<int>, std::forward_list<int>>;
TYPED_TEST_SUITE(SplicingTests, Spliceable);

TYPED_TEST(SplicingTests, concepts) {
  EXPECT_TRUE((spliceable_range<TypeParam>));
  EXPECT_TRUE((spliceable_with_range<TypeParam, TypeParam>));

  TypeParam range{};
  const auto bb = before_begin(range);

  EXPECT_TRUE((std::sentinel_for<std::decay_t<decltype(bb)>,
                                 ranges::iterator_t<TypeParam>>));
  ASSERT_NE(bb, range.end());

  EXPECT_TRUE((left_limit_of<std::decay_t<decltype(bb)>, TypeParam>));
  EXPECT_TRUE((left_limit_of<std::decay_t<decltype(ranges::begin(range))>,
                                          TypeParam>));

  EXPECT_TRUE((right_limit_of<std::decay_t<decltype(ranges::begin(range))>,
                                          TypeParam>));
  EXPECT_TRUE((right_limit_of<std::decay_t<decltype(ranges::end(range))>,
                                          TypeParam>));

  EXPECT_EQ(after(range, bb), ranges::begin(range));

  range.push_front(42);
  EXPECT_EQ(after(range, bb), ranges::begin(range));
}
