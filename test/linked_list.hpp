#pragma once
#include <cstddef>
#include <memory>

#include "enranged/limits.hpp"

using std::size_t;

/**
 * @brief The simplest linked list for testing purposes
 **/
template <typename T, typename Allocator = std::allocator<T>>
class linked_list {
private:
  struct node_t {
    T value;
    node_t* next;
  };

  using allocator_t =
    std::allocator_traits<Allocator>::template rebind_alloc<node_t>;

public:
  using value_type = T;

  linked_list(const size_t size): head_(nullptr) {
    for (size_t i = 0; i < size; ++i)
      emplace_front();
  }

  linked_list() noexcept: linked_list(0) {}
  ~linked_list() noexcept { delete_all(); }

  linked_list(const linked_list&) = delete;
  linked_list& operator=(const linked_list&) = delete;

  linked_list(linked_list&& rhs) noexcept {
    delete_all();
    move_from(std::move(rhs));
  }

  linked_list& operator=(linked_list&& rhs) noexcept {
    if (this != std::addressof(rhs)) {
      delete_all();
      move_from(std::move(rhs));
    }
    return *this;
  }

  template <typename... Args>
  void emplace_front(Args&&... args) {
    head_ = new (allocator_.allocate(1)) node_t {
      .value = T{std::forward<Args>(args)...},
      .next = head_
    };

    if (!last_) last_ = head_;
  }

  class iterator {
  public:
    using value_type = T;
    using difference_type = std::ptrdiff_t;

    iterator() noexcept = default;
    bool operator==(const iterator&) const = default;

    T& operator*() const noexcept {
      return ptr_->value;
    }

    iterator& operator++() noexcept {
      ptr_ = ptr_->next;
      return *this;
    }

    iterator operator++(int) noexcept {
      iterator result{ptr_};
      ++*this;
      return result;
    }

  private:
    iterator(node_t* const ptr) noexcept: ptr_(ptr) {}

    node_t* ptr_;
    friend class linked_list;
  };

  iterator begin() noexcept {
    return { head_ };
  }

  iterator end() noexcept {
    return { nullptr };
  }

  struct front_sentinel {
    bool operator==(const iterator&) const noexcept {
      return false;
    }
  };

  front_sentinel before_begin() noexcept {
    return {};
  }

  iterator last() noexcept {
    return { last_ };
  }

  template <enranged::left_limit_of<linked_list> P,
            enranged::left_limit_of<linked_list> L>
  void cosplice(const P pos, linked_list& other, const L lt,
                const iterator rt) noexcept {
    node_t* left_node;
    if constexpr (std::same_as<L, iterator>) {
      left_node = lt.ptr_->next;
      lt.ptr_->next = rt.ptr_->next;
    }
    else {
      left_node = other.head_;
      other.head_ = rt.ptr_->next;
    }

    if constexpr (std::same_as<P, iterator>) {
      rt.ptr_->next = pos.ptr_->next;
      pos.ptr_->next = left_node;
    }
    else {
      rt.ptr_->next = head_;
      head_ = left_node;
    }
  }

  template <enranged::left_limit_of<linked_list> P,
            enranged::left_limit_of<linked_list> I>
  void cosplice(const P pos, linked_list& other, const I it) noexcept {
    cosplice(pos, other, it, enranged::after(other, it));
  }

private:
  void delete_all() noexcept {
    auto node = head_;
    while (node) {
      const auto next_node = node->next;
      node->~node_t();
      allocator_.deallocate(node, 1);
      node = next_node;
    }

    head_ = last_ = nullptr;
  }

  void move_from(linked_list&& rhs) {
    std::swap(head_, rhs.head_);
    std::swap(last_, rhs.last_);

    allocator_ = std::move(rhs.allocator_);
  }

  node_t* head_ = nullptr;
  node_t* last_ = nullptr;

  allocator_t allocator_;

  friend class iterator;
};
