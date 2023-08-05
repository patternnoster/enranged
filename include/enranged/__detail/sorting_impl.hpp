#pragma once
#include <concepts>
#include <cstdint>
#include <functional>
#include <iterator>
#include <type_traits>
#include <utility>

#include "../splicing.hpp"

#include "flat_list.hpp"

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
  // We assume left != last, since middle must be in the corange
  if (middle == last) [[unlikely]] return last;

  auto rhs = ranges::next(middle);  // First element on the right
  const auto end = ranges::next(last);

  // Invariant #1: rhs != end (forward implies equality preserving)

  // Some optimistic thinking: what if everything is already sorted?
  if (!comp(*rhs, *middle)) return last;

  // Our optimistic thinking has given us:
  // Invariant #2: *rhs < *middle

  auto lhs = after(range, left); // First unsorted element on the left

  // We need a separate splice in case *rhs compares less than the
  // front element
  if (comp(*rhs, *lhs)) { /* <= (lhs == middle) */
    // We need to put some part of the right side front, figure out
    // how big of a part that is
    ranges::iterator_t<R> rhs_next = ranges::next(rhs);
    for (; rhs_next != end && comp(*rhs_next, *lhs); rhs = rhs_next++);

    cosplice(range, left, middle, rhs);

    if (rhs_next == end) return middle;
    if (lhs == middle || !comp(*rhs_next, *middle)) return last;

    rhs = rhs_next;
  }

  // Invariant #3: lhs != middle
  // Invariant #4: *lhs <= *rhs

  for (;;) {
    // Find the first left-hand side element that is greater than
    // *rhs. Because of invariant #2, such element always exists
    ranges::iterator_t<R> lhs_next = ranges::next(lhs);
    for (; !comp(*rhs, *lhs_next); lhs = lhs_next++);

    // Now *lhs <= *rhs < *lhs_next. Find out, how big of a part
    // following rhs we can splice in-between them
    ranges::iterator_t<R> rhs_next = ranges::next(rhs);
    for (; rhs_next != end && comp(*rhs_next, *lhs_next); rhs = rhs_next++);

    cosplice(range, lhs, middle, rhs);

    if (rhs_next == end) return middle;
    if (lhs_next == middle || !comp(*rhs_next, *middle)) return last;

    lhs = lhs_next;
    rhs = rhs_next;
  }
}

template <typename R, left_limit_of<R> L, typename Comp>
constexpr ranges::borrowed_iterator_t<R> insertion_sort_splice
  (R&& range, const L left_limit, size_t size, const Comp comp) {
  auto first = after(range, left_limit);
  if (size < 2) [[unlikely]] return first;

  auto lhs = first;  // Last sorted

  // Deal with the first two elements separately to avoid an
  // unnecessary comparison
  auto rhs = ranges::next(lhs);
  if (!comp(*rhs, *lhs)) lhs = rhs;
  else {
    cosplice(range, left_limit, lhs);
    first = rhs;
  }

  size-= 2;
  for (; size; --size) {
    rhs = ranges::next(lhs);
    // First determine the position of where we can put the rhs

    if (!comp(*rhs, *lhs)) {
      // Here *lhs <= *rhs and we can just fast-forward
      lhs = rhs;
      continue;
    }

    // NB: lhs won't change this time from now one

    /* Check if rhs should be put to the very front (note that since
     * we dealt with the first element separately, lhs != first, so
     * this is not a waste of a comparison anyway */
    if (comp(*rhs, *first)) {
      cosplice(range, left_limit, lhs);
      first = rhs;
      continue;
    }

    // Okay, so *first <= *rhs < *lhs, so we can iterate to find the
    // last element on the (sorted left) that is <= *rhs
    auto pos = first;
    for (auto pos_next = ranges::next(pos); !comp(*rhs, *pos_next);
         pos = pos_next++);

    cosplice(range, pos, lhs);
  }

  return lhs;
}

template <typename R, left_limit_of<R> L, typename Comp>
constexpr ranges::borrowed_iterator_t<R> merge_sort_splice
  (R&& range, const L left, const size_t size, const Comp comp) {
  constexpr size_t MergeThreshold = 4; // Use insertions if smaller
                                       // (must be a power of 2)

  /* Let L = ceil(log2(size+1)) and S(k) = size >> (L-k).
   * At step k we assume that the first S(k) elements are already
   * sorted. We apply merge_sort recursively to the next S(k+1)-S(k)
   * (which equals either S(k) or S(k)+1) elements, then we
   * inplace_merge the result with the first elements and move to the
   * next step.
   * This approach gives the most balanced division. Obviously, after
   * step L-1 is finished, the range is sorted.
   * If T = log2(MergeThreshold), then we can apply insertion sort to
   * the first S(T) elements and then start with k=T */
  const size_t max_steps = 64 - std::countl_zero(uint64_t(size)); // L
  constexpr size_t first_step = std::countr_zero(MergeThreshold); // T

  size_t l_cnt = max_steps <= first_step ? size
    : size >> (max_steps - first_step);
  auto last_sorted =
    __detail::insertion_sort_splice(std::forward<R>(range), left, l_cnt, comp);

  // Invariant: [begin(range), last_sorted] is already sorted and
  // contains l_cnt elements
  for (size_t step = first_step; step < max_steps; ++step) {
    const size_t to_sort = (size >> (max_steps - step - 1)) - l_cnt;

    const auto last_sorted_right =
      __detail::merge_sort_splice(std::forward<R>(range),
                                  last_sorted, to_sort, comp);

    last_sorted =
      __detail::coinplace_merge_splice(std::forward<R>(range), left,
                                       last_sorted, last_sorted_right, comp);

    l_cnt+= to_sort;
  }

  return last_sorted;
}

template <size_t _max_buckets,
          spliceable_range R, left_limit_of<R> L1, right_limit_of<R> L2,
          typename EqRel, typename Comp>
constexpr std::pair<size_t, ranges::borrowed_iterator_t<R>> bucket_sort_splice
  (R&& range, const L1 left, const L2 end, const EqRel is_eq, const Comp comp) {
  static_assert(_max_buckets > 0);
  using flat_list_t =
    flat_list<std::pair<size_t, ranges::iterator_t<R>>, _max_buckets>;

  auto lhs = after(range, left);  // Rightmost bucketed
  if (lhs == end) return std::make_pair(0, lhs);
  // Okay, that was nasty, but now we know the range has something

  flat_list_t memory;

  /* First, traverse the range to fill the buckets up. Our flat_list
   * contains the size of the bucket and an iterator to its last
   * element.
   * Invariant: if a bucket with b precedes the one with c in the
   * range then c >= b */

  // The first element always has its own bucket
  auto last_buck = memory.emplace_after(memory.before_begin(), 1, lhs);

  // If the range has more equivalency classes than _max_buckets, we
  // will put them in the last bucket and inplace_merge later
  bool last_buck_dirty = false;

  for (auto it = ranges::next(lhs); it != end;) {
     // We deal with the last bucket separately
    if (is_eq(*it, *last_buck->second)) {
      // No need for splicing, just fast forward
      ++last_buck->first;
      last_buck->second = it;
      lhs = it++;
      continue;
    }

    const bool can_add_buckets = memory.size() < _max_buckets;

    /* Okay, the element is not in the last bucket, but we might still
     * get away with no splicing: if the element is greater than the
     * representative, it cannot be in any bucket preceding this one
     * because of the invariant and the consistency. Therefore we will
     * need to start another bucket right here */
    if (comp(*last_buck->second, *it)) {
      if (can_add_buckets)
        last_buck = memory.emplace_after(last_buck, 1, it);
      else {
        // Put the unbucketed in the last bucket still (note that the
        // last bucket cannot change after the maximum is reached)
        ++last_buck->first;
        last_buck_dirty = true;
      }

      lhs = it++;
      continue;
    }

    // Fine, we need to check other buckets and splice. First decide
    // how many elements go, so we can optimize the splicing a little
    size_t size_to_bucket = 1;
    auto it_last = it;
    auto it_next = ranges::next(it);
    for (; it_next != end && is_eq(*it, *it_next); it_last = it_next++)
      ++size_to_bucket;

    // Now iterate the buckets to either find the proper one or the
    // position to put a new bucket after
    bool need_new_bucket = true;
    auto buck_it = memory.before_begin();
    for (auto buck_it_next = memory.begin(); buck_it_next != last_buck;
         buck_it = buck_it_next++) {
      if (is_eq(*it, *buck_it_next->second)) {
        // Found the proper bucket
        need_new_bucket = false;
        buck_it = buck_it_next;
        break;
      }

      /* Wrong bucket, same logic: if *it is less than the
       * representative, new bucket goes before this one.
       * Note that since the representative of the last bucket is >=
       * *it, this will always be true for the last bucket in case of
       * consistency requirement */
      if (comp(*it, *buck_it_next->second))
        break;
    }

    it = it_next;

    if (need_new_bucket) {
      if (!can_add_buckets) {
        // There is too many buckets already: keep the unbucketed
        // elements in the last bucket for now
        last_buck->first+= size_to_bucket;
        last_buck_dirty = true;

        lhs = it_last;
        continue;
      }

      if (buck_it == memory.before_begin()) {
        // Less or equal to all the buckets
        memory.emplace_after(buck_it, size_to_bucket, it_last);
        cosplice(range, left, lhs, it_last);
        continue;
      }
      buck_it = memory.emplace_after(buck_it, 0, buck_it->second);
    }

    cosplice(range, buck_it->second, lhs, it_last);
    buck_it->first+= size_to_bucket;
    buck_it->second = it_last;
  }

  return { 0, after(range, left) };
}

} // namespace enranged::__detail
