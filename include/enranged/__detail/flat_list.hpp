#pragma once
#include <bit>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

/**
 * @file
 * A helper pseudo-list structure for algorithm implementations
 *
 * @author    patternnoster@github
 * @copyright 2023, under the MIT License (see /LICENSE for details)
 **/

namespace enranged::__detail {

using std::byte;

/**
 * @brief The smallest unsigned integral type capable of holding
 *        values less or equal to _upper
 **/
template <uint64_t _upper, int _free_bits = std::countl_zero(_upper)>
using min_unsigned_t_for =
  std::conditional_t<_free_bits >= 56, uint8_t,
                     std::conditional_t<_free_bits >= 48, uint16_t,
                                        std::conditional_t<_free_bits >= 32,
                                                           uint32_t,
                                                           uint64_t>>>;

/**
 * @brief A simple pseudo-list of limited size implemented over a
 *        contiguous array
 **/
template <typename T, size_t _max_size>
class flat_list {
public:
  using pos_t = min_unsigned_t_for<_max_size>;

  flat_list() noexcept {
    links_[0] = _max_size;  // Marks the end
  }

  ~flat_list() noexcept {
    clear();
  }

  size_t size() const noexcept {
    return size_;
  }

  void clear() noexcept {
    auto it = reinterpret_cast<T*>(data_);
    const auto end = it + size_;

    while (it != end) it++->~T();
    size_ = 0;
  }

private:
  size_t size_ = 0;
  pos_t links_[_max_size + 1]; // links_[i] = the index of the next after
                               // (i-1)-th element (first if i == 0)

  alignas(T) byte data_[sizeof(T) * _max_size];
};

} // namespace enranged::__detail
