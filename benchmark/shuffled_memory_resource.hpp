#pragma once
#include <array>
#include <cstdlib>
#include <random>

/**
 * @brief A test memory resource (for benchmarking only), that
 *        allocates small blocks of memory in random order to force
 *        cache misses
 **/
template <size_t _max_size>
class shuffled_memory_resource {
public:
  constexpr static size_t max_alloc = 32;

  shuffled_memory_resource() {
    base_ptr_ = std::malloc(_max_size * max_alloc);
    for (size_t i = 0; i < _max_size; ++i)
      ptrs_[i] = static_cast<std::byte*>(base_ptr_) + i * max_alloc;

    std::random_device rd;
    std::shuffle(ptrs_.begin(), ptrs_.end(), std::mt19937{rd()});
  }

  ~shuffled_memory_resource() {
    std::free(base_ptr_);
  }

  shuffled_memory_resource(const shuffled_memory_resource&) = delete;
  shuffled_memory_resource(shuffled_memory_resource&&) = delete;
  shuffled_memory_resource& operator=(const shuffled_memory_resource&) = delete;
  shuffled_memory_resource& operator=(shuffled_memory_resource&&) = delete;

  void* allocate() noexcept {
    return ptrs_[idx_++];
  }

  void reset() noexcept {
    idx_ = 0;
  }

private:
  void* base_ptr_;

  std::array<void*, _max_size> ptrs_;
  size_t idx_ = 0;
};
