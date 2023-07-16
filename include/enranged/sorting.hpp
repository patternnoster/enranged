#pragma once
#include <concepts>
#include <functional>
#include <iterator>

#include "splicing.hpp"

#include "__detail/sorting_impl.hpp"

/**
 * @file
 * Additional sorting algorithms for certain kinds of ranges
 *
 * @author    patternnoster@github
 * @copyright 2023, under the MIT License (see /LICENSE for details)
 **/

namespace enranged {

/**
 * @brief The concept of a range that can be sorted by splicing with
 *        the provided strict weak order
 *
 * A predicate (denoted by x<y with its negation !(x<y) denoted by
 * x>=y) is a strict weak order, iff for all x, y and z:
 * (1) x >= x;
 * (2) if x < y then y >= x;
 * (3) if x >= y and y >= z then x >= z.
 *
 * Note that the C++20 standard gives this definition in a different
 * form, namely:
 * (1) x >= x;
 * (2') if x < y and y < z then x < z;
 * (3') if (x >= y & y >= x) & (y >= z & z >= y) then x >= z & z >= x.
 *
 * One can easily prove that both forms are equivalent (but the first
 * one looks simpler in my humble opinion, I have no idea why the
 * standard decided to go with the latter monstrosity).
 *
 * @note  This concept is different from std::sortable as it doesn't
 *        require the iterators to be permutable or even
 *        indirectly_movable, since there is no moving elements in
 *        splicing-based algorithms
 **/
template <typename R,
          typename Comp = ranges::less, typename Proj = std::identity>
concept splice_sortable_range = spliceable_range<R>
  && std::indirect_strict_weak_order<Comp,
                                     std::projected<ranges::iterator_t<R>,
                                                    Proj>>;

/**
 * @brief  Given a subrange (left, right] of a spliceable range and an
 *         iterator mid from that subrange, assumes the subranges
 *         (left, mid] and (mid, right] are sorted, performs a stable
 *         inplace splice-based merge into one sorted subrange (left,
 *         result], and returns result
 *
 * This version is dual to the regular std::inplace_merge(), which,
 * given iterators a, b and c, performs the merging of ranges [a, b)
 * and [b, c) instead of coranges (a, b] and (b, c] like
 * here. Furthermore, this version returns the last element of the
 * resulting range and not the unchanged upper bound (which makes the
 * returned value meaningful)
 *
 * @tparam Comp must be a strict weak order (see above)
 * @param  left must be a valid left limit of the given range (i.e., a
 *         front sentinel or a dereferenceable iterator)
 * @param  mid must be a dereferenceable iterator in (left, right]
 * @param  right must be a dereferenceable iterator of the given range,
 *         such that (left, right] is a valid corange (in particular,
 *         left != right)
 * @return The iterator to the last element of the sorted subrange
 * @note   The behaviour is undefined if either of (left, mid] or
 *         (mid, right] are not sorted
 **/
template <spliceable_range R, left_limit_of<R> L,
          typename Comp = ranges::less, typename Proj = std::identity>
  requires(splice_sortable_range<R, Comp, Proj>)
constexpr ranges::borrowed_iterator_t<R> coinplace_merge_splice
  (R&& range, const L left,
   const ranges::iterator_t<R> mid, const ranges::iterator_t<R> right,
   const Comp comp = {}, const Proj proj = {}) {
  return
    __detail::coinplace_merge_splice(std::forward<R>(range), left, mid, right,
                                     __detail::project_predicate(comp, proj));
}

/**
 * @brief  Performs a splice-based version of the stable insertion
 *         sorting algorithm on the corange (left, left + count] and
 *         returns an iterator to its last element
 * @tparam Comp must be a strict weak order (see above)
 * @param  left must be a valid left limit of the given range (i.e., a
 *         front sentinel or a dereferenceable iterator)
 * @param  count must not be greater than the number of elements
 *         following left in the given range
 * @return An iterator to the last element of the sorted corange (or
 *         after(range, left) if count is zero)
 * @note   Insertion sort works best on small or almost sorted ranges,
 *         otherwise a different algorithm should be chosen
 **/
template <spliceable_range R, left_limit_of<R> L,
          typename Comp = ranges::less, typename Proj = std::identity>
  requires(splice_sortable_range<R, Comp, Proj>)
constexpr ranges::borrowed_iterator_t<R> insertion_sort_splice
  (R&& range, const L left, const size_t count,
   const Comp comp = {}, const Proj proj = {}) {
  return
    __detail::insertion_sort_splice(std::forward<R>(range), left, count,
                                    __detail::project_predicate(comp, proj));
}

} // namespace enranged
