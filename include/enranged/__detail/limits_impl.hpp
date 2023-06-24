#pragma once
#include <cstddef>
#include <iterator>
#include <ranges>

/**
 * @file
 * Implementation details for range limits
 *
 * @author    patternnoster@github
 * @copyright 2023, under the MIT License (see /LICENSE for details)
 **/

namespace enranged {

namespace ranges = std::ranges;
using std::size_t;

namespace __detail {

template <typename R>
concept has_before_begin = requires(R obj) {
  { obj.before_begin() } noexcept -> std::sentinel_for<ranges::iterator_t<R>>;
};

template <typename R>
concept has_last = requires(R obj) {
  { obj.last() } -> std::same_as<ranges::iterator_t<R>>;
};

} // namespace __detail
} // namespace enranged
