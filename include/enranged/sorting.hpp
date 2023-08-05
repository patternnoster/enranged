#pragma once
#include <concepts>
#include <functional>
#include <iterator>
#include <ranges>

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
 * @brief  Given a spliceable corange (a, b] and its iterator mid,
 *         assumes the subranges (a, mid] and (mid, b] are sorted,
 *         performs a stable inplace splice-based merge into one
 *         sorted range and returns an iterator to its last element
 * @tparam Comp must be a strict weak order (see above)
 * @param  mid must be a dereferenceable iterator in range
 * @return An iterator to the last element of the given range after
 *         merge (i.e., last(range))
 * @note   The behaviour is undefined if either (before_begin(range),
 *         mid] or (mid, last(range)] are not sorted
 **/
template <spliceable_range R,
          typename Comp = ranges::less, typename Proj = std::identity>
  requires(corange<R> && splice_sortable_range<R, Comp, Proj>)
constexpr ranges::borrowed_iterator_t<R> coinplace_merge_splice
  (R&& range, const ranges::iterator_t<R> mid,
   const Comp comp = {}, const Proj proj = {}) {
  return
    coinplace_merge_splice(std::forward<R>(range), before_begin(range), mid,
                           last(range), comp, proj);
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

/**
 * @brief  Performs a splice-based version of the stable insertion
 *         sorting algorithm on the given sized range and returns an
 *         iterator to its last element
 * @tparam Comp must be a strict weak order (see above)
 * @return An iterator to the last element of the range (or equal to
 *         end(range) if the range is empty)
 * @note   Insertion sort works best on small or almost sorted ranges,
 *         otherwise a different algorithm should be chosen
 **/
template <spliceable_range R,
          typename Comp = ranges::less, typename Proj = std::identity>
  requires(ranges::sized_range<R> && splice_sortable_range<R, Comp, Proj>)
constexpr ranges::borrowed_iterator_t<R> insertion_sort_splice
  (R&& range, const Comp comp = {}, const Proj proj = {}) {
  return insertion_sort_splice(std::forward<R>(range), before_begin(range),
                               ranges::size(range), comp, proj);
}

/**
 * @brief  Performs a cache-friendly splice-based version of the stable
 *         merge sorting algorithm on the corange (left, left + count]
 *         and returns an iterator to its last element
 * @tparam Comp must be a strict weak order (see above)
 * @param  left must be a valid left limit of the given range (i.e., a
 *         front sentinel or a dereferenceable iterator)
 * @param  count must not be greater than the number of elements
 *         following left in the given range
 * @return An iterator to the last element of the sorted corange (or
 *         after(range, left) if count is zero)
 **/
template <spliceable_range R, left_limit_of<R> L,
          typename Comp = ranges::less, typename Proj = std::identity>
  requires(splice_sortable_range<R, Comp, Proj>)
constexpr ranges::borrowed_iterator_t<R> merge_sort_splice
  (R&& range, const L left, const size_t count,
   const Comp comp = {}, const Proj proj = {}) {
  return __detail::merge_sort_splice(std::forward<R>(range), left, count,
                                     __detail::project_predicate(comp, proj));
}

/**
 * @brief  Performs a cache-friendly splice-based version of the stable
 *         merge sorting algorithm on the given sized range and
 *         returns an iterator to its last element
 * @tparam Comp must be a strict weak order (see above)
 * @return An iterator to the last element of the range (or equal to
 *         end(range) if the range is empty)
 **/
template <spliceable_range R,
          typename Comp = ranges::less, typename Proj = std::identity>
  requires(ranges::sized_range<R> && splice_sortable_range<R, Comp, Proj>)
constexpr ranges::borrowed_iterator_t<R> merge_sort_splice
  (R&& range, const Comp comp = {}, const Proj proj = {}) {
  return merge_sort_splice(std::forward<R>(range), before_begin(range),
                           ranges::size(range), comp, proj);
}

/**
 * @brief  Performs a splice-based version of the stable bucket sorting
 *         algorithm on the open interval (left, right) in the given
 *         range, using a strict weak order and an equivalence
 *         relation that is consistent with it.
 *
 * A predicate (denoted by x~y) is an equivalence relation if (x~x),
 * (x~y => y~x) and (x~y & y~z => x~z) for all x, y and z.
 *
 * We say that an equivalence relation is consistent with a strict
 * weak order (denoted by x<y with its negation !(x<y) denoted by
 * x>=y) iff for all x, y, a, b:
 * - if (x>=y and y>=x) then x~y
 * - if (x<y & x~a & y~b) then (a<b or a~b)
 *
 * Equivalently, the relation x<'y <=> x<y & !(x~y) induces a (strict)
 * total order on equivalency classes.
 *
 * Put simply, consistency means that any pair of elements from two
 * different equivalence classes (i.e. buckets) compare the same. An
 * example of such a relation on positive integers, consistent with
 * the natural order (<), is: x~y <=> (x>>k) == (y>>k) for some k.
 *
 * To get the best performance, one should choose a relation that
 * gives not-too-many buckets rougly equal in size. As corner cases,
 * if none of the elements are equivalent, the algorithm degrades to
 * insertion_sort; if all elements are equivalent, it degrades to
 * merge_sort with an extra traversal of the entire range
 *
 * @tparam _max_buckets is the maximum number of equivalence classes
 *         used for the given interval
 * @tparam EqRel must be an equivalence relation consistent with Comp
 *         (see above)
 * @tparam Comp must be a strict weak order (see above)
 * @param  left must be a valid left limit of the given range (i.e., a
 *         front sentinel or a dereferenceable iterator)
 * @param  right must be a valid right limit of the given range (i.e.,
 *         a sentinel equal to end(range) or a dereferenceable
 *         iterator)
 * @return The size of the sorted interval and an iterator to its last
 *         element after sorting (or after(range, left) if the
 *         interval is empty)
 * @note   If the real number of buckets is bigger than _max_buckets,
 *         the algorithm will still work correctly but a little less
 *         efficiently, as it will require an additional inplace_merge
 **/
template <size_t _max_buckets = 32,
          spliceable_range R, left_limit_of<R> L1, right_limit_of<R> L2,
          typename EqRel, typename Proj1 = std::identity,
          typename Comp = ranges::less, typename Proj2 = std::identity>
  requires(_max_buckets > 0 && splice_sortable_range<R, Comp, Proj2>
           && std::indirect_equivalence_relation
              <EqRel, std::projected<ranges::iterator_t<R>, Proj1>>)
constexpr std::pair<size_t, ranges::borrowed_iterator_t<R>> bucket_sort_splice
  (R&& range, const L1 left, const L2 right,
   const EqRel rel, const Proj1 proj1 = {},
   const Comp comp = {}, const Proj2 proj2 = {}) {
  return __detail::bucket_sort_splice<_max_buckets>
    (std::forward<R>(range), left, right,
     __detail::project_predicate(rel, proj1),
     __detail::project_predicate(comp, proj2));
}

} // namespace enranged
