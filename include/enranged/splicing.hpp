#pragma once
#include <ranges>

#include "__detail/splicing_impl.hpp"

/**
 * @file
 * Tools for ranges that can be spliced (e.g. linked lists)
 *
 * @author    patternnoster@github
 * @copyright 2023, under the MIT License (see /LICENSE for details)
 **/

namespace enranged {

/**
 * @brief The concept of a range that can be naturally spliced,
 *        i.e. its subrange can be cheaply moved to the denoted place
 *        in the same range
 *
 * Examples of such ranges include std::list and std::forward_list
 * (and linked lists in general). This property allows for efficient
 * implementations of various algorithms that permute elements (like
 * sorting), without the random_access or even the bidirectional
 * requirements.
 *
 * Such ranges must either:
 *   define splice() methods with the same signature and semantics as
 *   std::list<T>::splice (putting a subrange [begin, end) or a given
 *   element before the given position) without invalidating any
 *   iterators;
 * or both:
 *   - define a noexcept method before_begin() returning a front
 *     sentinel (with the same semantics as in std::forward_list<T>
 *     but not necessarily of the iterator type)
 *   - and either
 *       define splice_after() methods with the same signature and
 *       semantics as std::forward_list<T>::splice_after (putting a
 *       subrange (begin, end) or the element following a given one
 *       after the given position) without invalidating any iterators;
 *     or
 *       define cosplice() methods with the same signature and
 *       semantics as explained below (putting a subrange (begin,
 *       end] or the element following a given one after the given
 *       position) without invalidating any iterators.
 *
 * In the second case, the splice_after() or cosplice() methods must
 * correctly accept the result of before_begin() (whether of same type
 * as an iterator or not) as both the position argument and the left
 * limit argument (or the iterator argument in the single element
 * splicing). Note that before_begin() can simply return the
 * default_front_sentinel as long as this requirement is met with
 * overloads
 *
 * For singly linked lists (or similar structures) the cosplice()
 * option should be preferred, as it allows for O(1) complexity in the
 * subrange case
 **/
template <typename R>
concept spliceable_range = ranges::forward_range<R>
  && (__detail::has_splice<R, R>
      || (__detail::has_before_begin<R> && (__detail::has_splice_after<R, R>
                                            || __detail::has_cosplice<R, R>)));

/**
 * @brief The concept of a range that can be naturally spliced with a
 *        subrange of another range (see spliceable_range for details)
 * @note  There may be additional constraints imposed on the ranges
 *        being spliced, that are not expressible with the language of
 *        concepts. For example std::list<T>::splice() requires that
 *        both the source and destination lists have equal allocators,
 *        i.e. src.get_allocator() == dst.get_allocator(). Violating
 *        requirements like that one may lead to undefined behaviour
 **/
template <typename R1, typename R2>
concept spliceable_with_range =
  ranges::forward_range<R1> && ranges::forward_range<R2>
  && (__detail::has_splice<R1, R2>
      || (__detail::has_before_begin<R1> && __detail::has_before_begin<R2>
          && (__detail::has_splice_after<R1, R2>
              || __detail::has_cosplice<R1, R2>)));

/**
 * @brief Moves the elements in the corange (lt, rt] of the source
 *        range after the specified position in the destination range
 *
 * The cosplice() methods are dual to the regular splice() ones (in a
 * sense that the latter move a range [a, b) before pos). They are
 * suitable for both singly and doubly linked lists, but unlike the
 * standard splice_after() (as in std::forward_list<T>) can have O(1)
 * complexity.
 *
 * @param pos must be a valid left limit of dst_range (i.e., a front
 *        sentinel or a dereferenceable iterator)
 * @param lt must be a valid left limit of src_range (i.e., a front
 *        sentinel or a dereferenceable iterator)
 * @param rt must be a dereferenceable iterator of src_range, such
 *        that (lt, rt] is a valid corange (in particular, lt != rt)
 * @note  The behaviour is undefined if pos is in (lt, rt] or is equal
 *        to lt (when dst_range and src_range are the same)
 **/
template <ranges::forward_range D, ranges::forward_range S,
          left_limit_of<D> P, left_limit_of<S> L>
  requires(spliceable_with_range<D, S>)
constexpr void cosplice(D&& dst_range, const P pos, S&& src_range, const L lt,
                        const ranges::iterator_t<S> rt) {
  if constexpr (__detail::has_cosplice<D, S>)
    dst_range.cosplice(pos, src_range, lt, rt);
  else if constexpr (__detail::has_splice<D, S>)
    dst_range.splice(after(dst_range, pos),
                     src_range, after(src_range, lt), ranges::next(rt));
  else
    /* splice_after() should always be the least priority option
     * because it may require traversal of the source range to
     * identify its last element (which gives O(n) complexity as in
     * std::forward_list) */
    dst_range.splice_after(pos, src_range, lt, ranges::next(rt));
}

/**
 * @brief Moves the element immediately following the one pointed to
 *        by (it) in the source range after the specified position in
 *        the destination range
 * @param pos must be a valid left limit of dst_range (i.e., a front
 *        sentinel or a dereferenceable iterator)
 * @param it must be a valid left limit of src_range (i.e., a front
 *        sentinel or a dereferenceable iterator), such that
 *        after(src_range, it) is dereferenceable
 * @note  The behaviour is undefined if pos is equal to (it) or to
 *        after(src_range, it) (when dst_range and src_range are the
 *        same)
 **/
template <ranges::forward_range D, ranges::forward_range S,
          left_limit_of<D> P, left_limit_of<S> I>
  requires(spliceable_with_range<D, S>)
constexpr void cosplice(D&& dst_range, const P pos, S&& src_range, const I it) {
  if constexpr (__detail::has_cosplice<D, S>)
    dst_range.cosplice(pos, src_range, it);
  else if constexpr (__detail::has_splice_after<D, S>)
    dst_range.splice_after(pos, src_range, it);
  else
    dst_range.splice(after(dst_range, pos), src_range, after(src_range, it));
}

} // namespace enranged
