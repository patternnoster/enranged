#include <deque>
#include <forward_list>
#include <gtest/gtest.h>
#include <list>
#include <vector>

#include "enranged/limits.hpp"

#include "linked_list.hpp"

using namespace enranged;

template <typename T>
class LimitsTests: public ::testing::Test {};

using Ranges = ::testing::Types<std::vector<int>, std::deque<int>,
                                std::list<int>, std::forward_list<int>,
                                linked_list<int>>;
TYPED_TEST_SUITE(LimitsTests, Ranges);

void emplace_front(auto& cont, const auto elt) { cont.emplace_front(elt); }
void emplace_front(std::vector<int>& cont, const int elt) {
  cont.insert(cont.begin(), elt);
}

TYPED_TEST(LimitsTests, before_begin) {
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

  emplace_front(range, 42);
  EXPECT_EQ(after(range, bb), ranges::begin(range));
}

template <typename T>
class CorangeLimitsTests: public ::testing::Test {};

using Coranges = ::testing::Types<std::vector<int>, std::deque<int>,
                                  std::list<int>, linked_list<int>>;
TYPED_TEST_SUITE(CorangeLimitsTests, Coranges);

TYPED_TEST(CorangeLimitsTests, last) {
  TypeParam range{};
  EXPECT_TRUE((corange<TypeParam>));

  emplace_front(range, 42);
  emplace_front(range, 17);

  EXPECT_EQ(*last(range), 42);
  EXPECT_EQ(after(range, last(range)), ranges::end(range));
}
