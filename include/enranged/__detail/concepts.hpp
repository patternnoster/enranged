#pragma once
#include <iterator>
#include <ranges>

namespace ranges = std::ranges;

namespace enranged::__detail {

template <typename R>
concept has_splice = requires(R obj, ranges::iterator_t<R> it) {
  obj.splice(it, obj, it);
  obj.splice(it, obj, it, it);
};

template <typename R>
concept has_before_begin = requires(R obj) {
  { obj.before_begin() } noexcept -> std::sentinel_for<ranges::iterator_t<R>>;
};

template <typename R>
concept has_splice_after = requires(R obj, ranges::iterator_t<R> it) {
  obj.splice_after(it, obj, it);
  obj.splice_after(it, obj, it, it);
};

} // namespace enranged::__detail
