#pragma once
#include <concepts>
#include <functional>
#include <iterator>
#include <type_traits>
#include <utility>

#include "../splicing.hpp"

/**
 * @file
 * Implementation of sorting algorithms for certain kinds of ranges
 *
 * @author    patternnoster@github
 * @copyright 2023, under the MIT License (see /LICENSE for details)
 **/

namespace enranged::__detail {

template <typename Pred, typename Proj>
constexpr decltype(auto) project_predicate(Pred& pred, Proj& proj) noexcept {
  if constexpr (std::same_as<std::remove_cvref_t<Proj>, std::identity>
                && !std::is_member_pointer_v<std::decay_t<Pred>>)
    /* We can easily avoid an indirect call of the lambda in most
     * cases with this condition, looks much better in the call stack
     * this way */
    return pred;
  else
    return [&](auto&& lhs, auto&& rhs) noexcept {
        return std::invoke(pred,
                           std::invoke(proj, std::forward<decltype(lhs)>(lhs)),
                           std::invoke(proj, std::forward<decltype(rhs)>(rhs)));
    };
}

template <spliceable_range R, left_limit_of<R> L, typename Comp>
constexpr ranges::borrowed_iterator_t<R> coinplace_merge_splice
  (R&& range, const L left, const ranges::iterator_t<R> middle,
   const ranges::iterator_t<R> last, const Comp comp) {
  // We assume left != last, since middle must be in the corange
  if (middle == last) [[unlikely]] return last;

  auto rhs = ranges::next(middle);  // First element on the right
  const auto end = ranges::next(last);

  // Invariant #1: rhs != end (forward implies equality preserving)

  // Some optimistic thinking: what if everything is already sorted?
  if (!comp(*rhs, *middle)) return last;

  // Our optimistic thinking has given us:
  // Invariant #2: *rhs < *middle

  auto lhs = after(range, left); // First unsorted element on the left

  // We need a separate splice in case *rhs compares less than the
  // front element
  if (comp(*rhs, *lhs)) { /* <= (lhs == middle) */
    // We need to put some part of the right side front, figure out
    // how big of a part that is
    ranges::iterator_t<R> rhs_next = ranges::next(rhs);
    for (; rhs_next != end && comp(*rhs_next, *lhs); rhs = rhs_next++);

    cosplice(range, left, middle, rhs);

    if (rhs_next == end) return middle;
    if (lhs == middle || !comp(*rhs_next, *middle)) return last;

    rhs = rhs_next;
  }

  // Invariant #3: lhs != middle
  // Invariant #4: *lhs <= *rhs

  for (;;) {
    // Find the first left-hand side element that is greater than
    // *rhs. Because of invariant #2, such element always exists
    ranges::iterator_t<R> lhs_next = ranges::next(lhs);
    for (; !comp(*rhs, *lhs_next); lhs = lhs_next++);

    // Now *lhs <= *rhs < *lhs_next. Find out, how big of a part
    // following rhs we can splice in-between them
    ranges::iterator_t<R> rhs_next = ranges::next(rhs);
    for (; rhs_next != end && comp(*rhs_next, *lhs_next); rhs = rhs_next++);

    cosplice(range, lhs, middle, rhs);

    if (rhs_next == end) return middle;
    if (lhs_next == middle || !comp(*rhs_next, *middle)) return last;

    lhs = lhs_next;
    rhs = rhs_next;
  }
}

template <typename R, left_limit_of<R> L, typename Comp>
constexpr ranges::borrowed_iterator_t<R> insertion_sort_splice
  (R&& range, const L left_limit, const size_t count, const Comp comp) {
  return after(range, left_limit);
}

} // namespace enranged::__detail
