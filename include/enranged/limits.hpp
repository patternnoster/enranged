#pragma once
#include <iterator>
#include <utility>

#include "__detail/concepts.hpp"

namespace enranged {

/**
 * @brief Returns a front sentinel that precedes the beginning of the
 *        given range
 *
 * This extends the idea of a "reverse end" to ranges that cannot be
 * naturally reversed (e.g., ranges that are not bidirectional or have
 * an unreachable end() sentinel), but can still somehow use the
 * concept of a pseudo-iterator pointing before the beggining (e.g.,
 * as in std::forward_list<T>::insert_after()).
 *
 * If the range implements a noexcept method before_begin() that
 * returns an std::sentinel_for its iterator, then the result of that
 * method will be used. Otherwise, default_front_sentinel is returned.
 *
 * A front sentinel s of a range r has the following semantic
 * requirements (that should be kept in mind when implementing
 * r.before_begin()):
 * - s cannot compare equal to any dereferenceable iterator of r or to
 *   an iterator that equals the end(r). In particular, in case of a
 *   common_range, s == end(r) must always return false;
 * - if s has the type of an iterator of r, then its increment must be
 *   either dereferenceable or equal to end(r). Furthermore, if r is
 *   a forward_range, then ++s == begin(r) must be true;
 * - s should not be dereferenced and, if r is not common_range, does
 *   not have to be equality comparable with end(r).
 **/
template <ranges::range R>
constexpr std::sentinel_for<ranges::iterator_t<R>> auto before_begin
  (R&&) noexcept;

/**
 * @brief The sentinel type that preceeds the beginning of the given
 *        range (see above for details)
 * @note  This type is always std::sentinel_for<iterator_t<R>>
 **/
template <ranges::range R>
using front_sentinel_t = decltype(before_begin(std::declval<R&>()));

/**
 * @brief The default implementation of the front sentinel for ranges
 *        that do not define a before_begin() method
 **/
struct default_front_sentinel_t: std::unreachable_sentinel_t {};
inline constexpr default_front_sentinel_t default_front_sentinel{};

template <ranges::range R>
constexpr std::sentinel_for<ranges::iterator_t<R>> auto before_begin
  (R&& range) noexcept {
  if constexpr (__detail::has_before_begin<R>)
    return std::forward<R>(range).before_begin();
  else
    return default_front_sentinel;
}

} // namespace enranged
