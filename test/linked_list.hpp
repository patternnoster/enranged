#pragma once
#include <cstddef>

using std::size_t;

/**
 * @brief The simplest linked list for testing purposes
 **/
template <typename T>
class linked_list {
private:
  struct node_t {
    T value;
    node_t* next;
  };

public:
  linked_list() noexcept = default;
  ~linked_list() noexcept { delete_all(); }

  linked_list(const linked_list&) = delete;
  linked_list& operator=(const linked_list&) = delete;

  linked_list(linked_list&& rhs) noexcept {
    delete_all();
    std::swap(head_, rhs.head_);
    std::swap(last_, rhs.last_);
  }

  linked_list& operator=(linked_list&& rhs) noexcept {
    if (this != std::addressof(rhs)) {
      delete_all();
      std::swap(head_, rhs.head_);
      std::swap(last_, rhs.last_);
    }
    return *this;
  }

  template <typename... Args>
  void emplace_front(Args&&... args) {
    head_ = new node_t {
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

private:
  void delete_all() noexcept {
    auto node = head_;
    while (node) {
      const auto next_node = node->next;
      delete node;
      node = next_node;
    }

    head_ = last_ = nullptr;
  }

  node_t* head_ = nullptr;
  node_t* last_ = nullptr;
  friend class iterator;
};
