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
 *     but not necessarily of the iterator type);
 *   - define splice_after() methods with the same signature and
 *     semantics as std::forward_list<T>::splice_after (putting a
 *     subrange (begin, end) or the element following a given one
 *     after the given position) without invalidating any iterators.
 *
 * In the second case, the splice_after() method must correctly accept
 * the result of before_begin() (whether of same type as an iterator
 * or not) as both the position argument and the left limit argument
 * (or the iterator argument in the single element splicing). Note
 * that before_begin() can simply return the default_front_sentinel as
 * long as this requirement is met with overloads
 **/
template <typename R>
concept spliceable_range = ranges::forward_range<R>
  && (__detail::has_splice<R>
      || (__detail::has_before_begin<R> && __detail::has_splice_after<R>));

} // namespace enranged
