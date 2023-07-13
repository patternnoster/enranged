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

template <typename R1, typename R2, typename P>
concept has_splice_at = requires(R1 obj, R2 src, P pos,
                                 ranges::iterator_t<R2> it,
                                 ranges::sentinel_t<R2> st) {
  obj.splice(pos, src, it);
  obj.splice(pos, src, it, it);
  obj.splice(pos, src, it, st);
};

template <typename R1, typename R2>
concept has_splice =
  has_splice_at<R1, R2, ranges::iterator_t<R1>>
  && has_splice_at<R1, R2, ranges::sentinel_t<R1>>;

template <typename R1, typename R2, typename P, typename L>
concept has_splice_after_at_left = requires(R1 obj, R2 src, P pos, L lt,
                                            ranges::iterator_t<R2> it,
                                            ranges::sentinel_t<R2> st) {
  obj.splice_after(pos, src, lt);
  obj.splice_after(pos, src, lt, it);
  obj.splice_after(pos, src, lt, st);
};

template <typename R1, typename R2, typename P>
concept has_splice_after_at =
  has_splice_after_at_left<R1, R2, P, front_sentinel_t<R2>>
  && has_splice_after_at_left<R1, R2, P, ranges::iterator_t<R2>>;

template <typename R1, typename R2>
concept has_splice_after =
  has_splice_after_at<R1, R2, front_sentinel_t<R1>>
  && has_splice_after_at<R1, R2, ranges::iterator_t<R1>>;

template <typename R1, typename R2, typename P, typename L>
concept has_cosplice_at_left = requires(R1 obj, R2 src, P pos, L lt,
                                        ranges::iterator_t<R2> it) {
  obj.cosplice(pos, src, lt);
  obj.cosplice(pos, src, lt, it);
};

template <typename R1, typename R2, typename P>
concept has_cosplice_at =
  has_cosplice_at_left<R1, R2, P, front_sentinel_t<R2>>
  && has_cosplice_at_left<R1, R2, P, ranges::iterator_t<R2>>;

template <typename R1, typename R2>
concept has_cosplice =
  has_cosplice_at<R1, R2, front_sentinel_t<R1>>
  && has_cosplice_at<R1, R2, ranges::iterator_t<R1>>;

} // namespace enranged::__detail
