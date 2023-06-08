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

  TypeParam range{};
  const auto bb = before_begin(range);

  EXPECT_TRUE((std::sentinel_for<std::decay_t<decltype(bb)>,
                                 ranges::iterator_t<TypeParam>>));
  ASSERT_NE(bb, range.end());
}
