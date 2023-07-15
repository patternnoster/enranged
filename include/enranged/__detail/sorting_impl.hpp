#pragma once
#include <concepts>
#include <functional>
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
  return last;
}

} // namespace enranged::__detail
