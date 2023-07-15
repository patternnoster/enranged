#pragma once
#include <concepts>
#include <functional>
#include <iterator>

#include "splicing.hpp"

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

} // namespace enranged
