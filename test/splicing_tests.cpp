#include <forward_list>
#include <gtest/gtest.h>
#include <list>

#include "enranged/splicing.hpp"

using namespace enranged;

template <typename T>
class SplicingTests: public ::testing::Test {};

using Spliceable = ::testing::Types<std::list<int>, std::forward_list<int>>;
TYPED_TEST_SUITE(SplicingTests, Spliceable);

TYPED_TEST(SplicingTests, concepts) {
  EXPECT_TRUE((spliceable_range<TypeParam>));
  EXPECT_TRUE((spliceable_with_range<TypeParam, TypeParam>));
}
