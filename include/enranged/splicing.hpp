#pragma once
#include <ranges>

#include "__detail/concepts.hpp"

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
 *   - define a noexcept method before_begin() returning a
 *     pseudo-iterator (with the same semantics as in
 *     std::forward_list<T>);
 *   - define splice_after() methods with the same signature and
 *     semantics as std::forward_list<T>::splice_after (putting a
 *     subrange (begin, end) or the element following a given one
 *     after the given position) without invalidating any iterators
 **/
template <typename R>
concept spliceable_range = ranges::forward_range<R>
  && (__detail::has_splice<R>
      || (__detail::has_before_begin<R> && __detail::has_splice_after<R>));

} // namespace enranged
