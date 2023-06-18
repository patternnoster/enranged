#pragma once
#include <iterator>
#include <ranges>

#include "../limits.hpp"

/**
 * @file
 * Implementation details for range splicing tools
 *
 * @author    patternnoster@github
 * @copyright 2023, under the MIT License (see /LICENSE for details)
 **/

namespace enranged::__detail {

template <typename R, typename P>
concept has_splice_at = requires(R obj, P pos,
                                 ranges::iterator_t<R> it,
                                 ranges::sentinel_t<R> st) {
  obj.splice(pos, obj, it);
  obj.splice(pos, obj, it, it);
  obj.splice(pos, obj, it, st);
};

template <typename R>
concept has_splice =
  has_splice_at<R, ranges::iterator_t<R>>
  && has_splice_at<R, ranges::sentinel_t<R>>;

template <typename R, typename P, typename L>
concept has_splice_after_at_left = requires(R obj, P pos, L lt,
                                            ranges::iterator_t<R> it,
                                            ranges::sentinel_t<R> st) {
  obj.splice_after(pos, obj, lt);
  obj.splice_after(pos, obj, lt, it);
  obj.splice_after(pos, obj, lt, st);
};

template <typename R, typename P>
concept has_splice_after_at =
  has_splice_after_at_left<R, P, front_sentinel_t<R>>
  && has_splice_after_at_left<R, P, ranges::iterator_t<R>>;

template <typename R>
concept has_splice_after =
  has_splice_after_at<R, front_sentinel_t<R>>
  && has_splice_after_at<R, ranges::iterator_t<R>>;

} // namespace enranged::__detail
