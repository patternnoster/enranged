# Spliced sorting

The library contains several sorting algorithms that use splicing instead of moving/swapping elements and can be applied to [spliceable ranges](#spliceable_range) (i.e., ranges like `std::list` or `std::forward_list` that define `splice()`/`splice_after()`/`cosplice()` methods). Hence, those algorithms do not require the range value type to be movable or the iterator to be random_access or even bidirectional (like the standard `std::ranges::sort` does).

## Members
### Concepts

| Name | Description |
|---|---|
| [**splice_sortable_range**](#splice_sortable_range) | the concept of a range that can be sorted by splicing with the provided strict weak order |

### Functions

| Name | Description |
|---|---|
| [**coinplace_merge_splice**](#coinplace_merge_splice) | given a subrange (left, right] of a spliceable range and an iterator mid from that subrange, assumes the subranges (left, mid] and (mid, right] are sorted, performs a stable inplace splice-based merge into one sorted subrange (left, result], and returns result |
| [**insertion_sort_splice**](#insertion_sort_splice) | performs a splice-based version of the stable insertion sorting algorithm on the corange (left, left + count] and returns an iterator to its last element |
| [**merge_sort_splice**](#merge_sort_splice) | performs a cache-friendly splice-based version of the stable merge sorting algorithm on the corange (left, left + count] and returns an iterator to its last element |

## Details
### splice_sortable_range
<sub>Defined in header [&lt;enranged/sorting.hpp&gt;](/include/enranged/sorting.hpp)</sub>
```c++
template <typename R,
          typename Comp = std::ranges::less, typename Proj = std::identity>
concept splice_sortable_range = spliceable_range<R>
  && std::indirect_strict_weak_order<Comp, std::projected<std::ranges::iterator_t<R>, Proj>>;
```
The concept of a range that can be sorted by splicing with the provided strict weak order.

A predicate (denoted by `x<y` with its negation `!(x<y)` denoted by `x>=y`) is a strict weak order, iff for all x, y and z:
1. `x >= x`;
2. if `x < y` then `y >= x`;
3. if `x >= y` and `y >= z` then `x >= z`.

Note that the C++20 standard gives this definition in a different form, namely:
1. `x >= x`;
2. if `x < y` and `y < z` then `x < z`;
3. if `(x >= y & y >= x) & (y >= z & z >= y)` then `x >= z & z >= x`.

One can easily prove that both forms are equivalent (but the first one looks simpler).

> [!NOTE]
> This concept is different from `std::sortable` as it doesn't require the iterators to be permutable or even indirectly_movable, since there is no moving elements in splicing-based algorithms.

---

### coinplace_merge_splice
<sub>Defined in header [&lt;enranged/sorting.hpp&gt;](/include/enranged/sorting.hpp)</sub>
```c++
template <spliceable_range R, left_limit_of<R> L,
          typename Comp = std::ranges::less, typename Proj = std::identity>
  requires(splice_sortable_range<R, Comp, Proj>)
constexpr std::ranges::borrowed_iterator_t<R> coinplace_merge_splice
  (R&& range, L left, std::ranges::iterator_t<R> mid, std::ranges::iterator_t<R> right,
   Comp comp = {}, Proj proj = {});
```
Given a subrange (left, right] of a spliceable range and an iterator mid from that subrange, assumes the subranges (left, mid] and (mid, right] are sorted, performs a stable inplace splice-based merge into one sorted subrange (left, result], and returns result.

This version is dual to the regular `std::inplace_merge()`, which, given iterators a, b and c, performs the merging of ranges [a, b) and [b, c) instead of coranges (a, b] and (b, c] like here. Furthermore, this version returns the last element of the resulting range and not the unchanged upper bound (which makes the returned value meaningful)

**Template parameters**

* `Comp` must be a strict weak order (see above)

**Parameters**

* `left` must be a valid left limit of the given range (i.e., a front sentinel or a dereferenceable iterator)
* `mid` must be a dereferenceable iterator in `(left, right]`
* `right` must be a dereferenceable iterator of the given range, such that `(left, right]` is a valid corange (in particular, `left != right`)

**Return value**

The iterator to the last element of the sorted subrange.

> [!NOTE]
> The behaviour is undefined if either of (left, mid] or (mid, right] are not sorted.

---

<sub>Defined in header [&lt;enranged/sorting.hpp&gt;](/include/enranged/sorting.hpp)</sub>
```c++
template <spliceable_range R,
          typename Comp = std::ranges::less, typename Proj = std::identity>
  requires(corange<R> && splice_sortable_range<R, Comp, Proj>)
constexpr std::ranges::borrowed_iterator_t<R> coinplace_merge_splice
  (R&& range, std::ranges::iterator_t<R> mid, Comp comp = {}, Proj proj = {});
```
Given a spliceable [**corange**](#corange) (a, b] and its iterator mid, assumes the subranges (a, mid] and (mid, b] are sorted, performs a stable inplace splice-based merge into one sorted range and returns an iterator to its last element.

**Template parameters**

* `Comp` must be a strict weak order (see above)

**Parameters**

* `mid` must be a dereferenceable iterator in `range`

**Return value**

An iterator to the last element of the given range after merge (i.e., [**last(range)**](#last)).

> [!NOTE]
> The behaviour is undefined if either ([**before_begin(range)**](#before_begin), mid] or (mid, [**last(range)**](#last)] are not sorted.

# Splicing

Splicing allows to cheaply reorder elements in a suitable range (e.g., linked list) without copying. The library formalizes this concept and introduces the notion of [cosplicing](#cosplice) that works with both singly and doubly linked lists but doesn't have the complexity penalty of `std::forward_list<T>::splice_after()`.

## Members
### Concepts

| Name | Description |
|---|---|
| [**spliceable_range**](#spliceable_range) | the concept of a range that can be naturally spliced, i.e. its subrange can be cheaply moved to the denoted place in the same range |
| [**spliceable_with_range**](#spliceable_with_range) | the concept of a range that can be naturally spliced with a subrange of another range |

### Functions

| Name | Description |
|---|---|
| [**cosplice**](#cosplice) | moves the elements in the corange (lt, rt] of the source range after the specified position in the destination range |

# Limits

A range is a half-open interval [begin, end) defined by an iterator to its first element and a sentinel. This library allows to define and manipulate intervals of other kinds as well (most importantly, half-closed intervals (before_begin, last] that we call [coranges](#corange)) which is especially useful when working with ranges that are not bidirectional (or common).

## Members
### Concepts

| Name | Description |
|---|---|
| [**corange**](#corange) | the concept of a range with the known last element |
| [**forward_corange**](#forward_corange) | the concept of a corange that is also an `std::forward_range` |
| [**left_limit_of**](#left_limit_of) | the concept of a type that can serve as a lower bound for a subrange of the given range (i.e. a dereferenceable iterator or a front sentinel) |
| [**right_limit_of**](#right_limit_of) | the concept of a type that can serve as an upper bound for a subrange of the given range (i.e. a dereferenceable iterator or a sentinel) |

### Helper types

| Name | Description |
|---|---|
| [**front_sentinel_t**](#front_sentinel_t) | the sentinel type that preceeds the beginning of the given range |

### Classes

| Name | Description |
|---|---|
| [**default_front_sentinel_t**](#default_front_sentinel_t) | the default implementation of the front sentinel for ranges that do not define a [**before_begin()**](#before_begin) method |

### Global variables

| Name | Description |
|---|---|
| [**default_front_sentinel**](#default_front_sentinel) | a (global constant) object of type [**default_front_sentinel_t**](#default_front_sentinel_t) that may serve as a universal (unreachable) front sentinel |

### Functions

| Name | Description |
|---|---|
| [**after**](#after) | returns an iterator to an element immediately following the given left limit in the given range |
| [**before_begin**](#before_begin) | returns a front sentinel that precedes the beginning of the given range |
| [**last**](#last) | returns an iterator to the last element of the given corange |
