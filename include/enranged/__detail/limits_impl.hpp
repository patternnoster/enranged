#pragma once
#include <cstddef>
#include <iterator>
#include <ranges>

namespace enranged {

namespace ranges = std::ranges;
using std::size_t;

namespace __detail {

template <typename R>
concept has_before_begin = requires(R obj) {
  { obj.before_begin() } noexcept -> std::sentinel_for<ranges::iterator_t<R>>;
};

} // namespace __detail
} // namespace enranged
