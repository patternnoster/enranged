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

  class iterator {
  public:
    using value_type = T;
    using difference_type = std::ptrdiff_t;

    iterator() noexcept = default;
    bool operator==(const iterator& other) const noexcept {
      return pos_ == other.pos_;  // Comparison between different
                                  // containers is UB anyway
    }

    T* operator->() const noexcept {
      return (reinterpret_cast<T*>(list_->data_) + pos_);
    }

    T& operator*() const noexcept {
      return *this->operator->();
    }

    iterator& operator++() noexcept {
      pos_ = list_->links_[pos_ + 1];
      return *this;
    }

    iterator operator++(int) noexcept {
      iterator result{*this};
      ++*this;
      return result;
    }

  private:
    iterator(flat_list* const list, const size_t pos) noexcept:
      list_(list), pos_(pos) {}

    flat_list* list_;
    size_t pos_;

    friend class flat_list;
  };

  /**
   * @brief  Constructs an element after the given iterator using the
   *         provided arguments. No iterators are invalidated. If the
   *         list already has _max_size elements or (it) is not
   *         before_begin() or dereferenceable, the behaviour is
   *         undefined
   * @return An iterator to the newly constructed element
   **/
  template <typename... Args>
  iterator emplace_after(const iterator& it, Args&&... args)
    noexcept(std::is_nothrow_constructible_v<T, Args...>) {
    new (static_cast<void*>(data_ + sizeof(T) * size_))
      T{std::forward<Args>(args)...};

    auto& it_link = links_[it.pos_ + 1];
    links_[size_ + 1] = it_link;
    it_link = pos_t(size_);

    return { this, size_++ };
  }

  iterator before_begin() noexcept {
    return { this, size_t(-1) };
  }

  iterator begin() noexcept {
    return { this, *links_ };
  }

  iterator end() noexcept {
    return { this, _max_size };
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
  friend class iterator;
};

} // namespace enranged::__detail
