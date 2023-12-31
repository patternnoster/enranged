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
| [**bucket_sort_splice**](#bucket_sort_splice) | performs a splice-based version of the bucket sorting algorithm on the open interval (left, right) in the given range, using a strict weak order and an equivalence relation that is weakly consistent with it. If the relation is (totally) consistent with the order, then the sorting is stable |
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

### bucket_sort_splice
<sub>Defined in header [&lt;enranged/sorting.hpp&gt;](/include/enranged/sorting.hpp)</sub>
```c++
template <size_t _max_buckets = 32,
          spliceable_range R, left_limit_of<R> L1, right_limit_of<R> L2,
          typename EqRel, typename Proj1 = std::identity,
          typename Comp = std::ranges::less, typename Proj2 = std::identity>
  requires(_max_buckets > 0 && splice_sortable_range<R, Comp, Proj2>
           && std::indirect_equivalence_relation
              <EqRel, std::projected<std::ranges::iterator_t<R>, Proj1>>)
constexpr std::pair<size_t, std::ranges::borrowed_iterator_t<R>> bucket_sort_splice
  (R&& range, L1 left, L2 right,
   EqRel rel, Proj1 proj1 = {}, Comp comp = {}, Proj2 proj2 = {});
```
Performs a splice-based version of the bucket sorting algorithm on the open interval (left, right) in the given range, using a strict weak order and an equivalence relation that is weakly consistent with it. If the relation is (totally) consistent with the order, then the sorting is stable.

A predicate (denoted by `x~y`) is an equivalence relation if `x~x`, `x~y` implies `y~x` and `x~y & y~z` implies `x~z` for all x, y and z.

We say that an equivalence relation is weakly consistent with a strict weak order (denoted by `x<y` with its negation `!(x<y)` denoted by `x>=y`) iff `x<y & x~a & y~b` implies `a<b` or `a~b` for all x, y, a, b. Or, equivalently, the relation `x<'y <=> x<y & !(x~y)` is a strict weak order.
Put simply, weak consistency means that any pair of elements from two different equivalence classes (i.e. buckets) compare the same. An example of such a relation on positive integers, weakly consistent with the natural order (<), is: `x~y <=> (x>>k) == (y>>k)` for some k.

An equivalence relation is (totally) consistent with a strict weak order iff it is weakly consistent with it and additionally `x>=y & y>=x` implies `x~y`. Equivalently, `x<'y` induces a (strict) total order on equivalency classes.
In the bit shift example above the relation is also (totally) consistent with the natural order.

To get the best performance, one should choose a relation that gives not-too-many buckets rougly equal in size. As corner cases, if none of the elements are equivalent, the algorithm degrades to [insertion sort](#insertion_sort_splice); if all elements are equivalent, it degrades to [merge sort](#merge_sort_splice) with an extra traversal of the entire range.

**Template parameters**

* `_max_buckets` is the maximum number of equivalence classes used for the given interval
* `EqRel` must be an equivalence relation weakly consistent with Comp (see above)
* `Comp` must be a strict weak order (see above)

**Parameters**

* `left` must be a valid left limit of the given range (i.e., a front sentinel or a dereferenceable iterator)
* `right` must be a valid right limit of the given range (i.e., a sentinel equal to **end(range)** or a dereferenceable iterator)

**Return value**

The size of the sorted interval and an iterator to its last element after sorting (or [**after(range, left)**](#after) if the interval is empty).

> [!NOTE]
> If the real number of buckets is bigger than _max_buckets, the algorithm will still work correctly but a little less efficiently, as it will require an additional [inplace merge](#coinplace_merge_splice).

> [!NOTE]
> The algorithm uses additional `_max_buckets * (sizeof(pair<size_t, iterator_t<R>>) + sizeof(T))` bytes of memory on the stack, where T is the minimal unsigned type capable of holding _max_buckets (e.g., `uint8_t` if it is <= 255). If that is too much stack memory, consider using the version that takes an allocator.

---

<sub>Defined in header [&lt;enranged/sorting.hpp&gt;](/include/enranged/sorting.hpp)</sub>
```c++
template <size_t _max_buckets = 32, typename Allocator,
          spliceable_range R, left_limit_of<R> L1, right_limit_of<R> L2,
          typename EqRel, typename Proj1 = std::identity,
          typename Comp = std::ranges::less, typename Proj2 = std::identity>
  requires(_max_buckets > 0 && splice_sortable_range<R, Comp, Proj2>
           && std::indirect_equivalence_relation
              <EqRel, std::projected<std::ranges::iterator_t<R>, Proj1>>)
constexpr std::pair<size_t, std::ranges::borrowed_iterator_t<R>> bucket_sort_splice
  (Allocator&& alloc, R&& range, L1 left, L2 right,
   EqRel rel, Proj1 proj1 = {}, Comp comp = {}, Proj2 proj2 = {});
```
Performs a splice-based version of the bucket sorting algorithm on the open interval (left, right) in the given range, using a strict weak order, an equivalence relation that is weakly consistent with it (see above for details) and a custom allocator for additional memory. If the relation is (totally) consistent with the order, then the sorting is stable.

**Template parameters**

* `_max_buckets` is the maximum number of equivalence classes used for the given interval
* `EqRel` must be an equivalence relation weakly consistent with Comp (see above)
* `Comp` must be a strict weak order (see above)

**Parameters**

* `left` must be a valid left limit of the given range (i.e., a front sentinel or a dereferenceable iterator)
* `right` must be a valid right limit of the given range (i.e., a sentinel equal to **end(range)** or a dereferenceable iterator)

**Return value**

The size of the sorted interval and an iterator to its last element after sorting (or [**after(range, left)**](#after) if the interval is empty).

> [!NOTE]
> If the real number of buckets is bigger than _max_buckets, the algorithm will still work correctly but a little less efficiently, as it will require an additional [inplace merge](#coinplace_merge_splice).

---

<sub>Defined in header [&lt;enranged/sorting.hpp&gt;](/include/enranged/sorting.hpp)</sub>
```c++
template <size_t _max_buckets = 32,
          spliceable_range R, typename EqRel, typename Proj1 = std::identity,
          typename Comp = std::ranges::less, typename Proj2 = std::identity>
  requires(_max_buckets > 0 && splice_sortable_range<R, Comp, Proj2>
           && std::indirect_equivalence_relation
              <EqRel, std::projected<std::ranges::iterator_t<R>, Proj1>>)
constexpr std::pair<size_t, std::ranges::borrowed_iterator_t<R>> bucket_sort_splice
  (R&& range, EqRel rel, Proj1 proj1 = {}, Comp comp = {}, Proj2 proj2 = {});
```
Performs a splice-based version of the bucket sorting algorithm on the given range, using a strict weak order and an equivalence relation that is weakly consistent with it (see above for details). If the relation is (totally) consistent with the order, then the sorting is stable.

**Template parameters**

* `_max_buckets` is the maximum number of equivalence classes used for the given range
* `EqRel` must be an equivalence relation weakly consistent with Comp (see above)
* `Comp` must be a strict weak order (see above)

**Return value**

The size of the range and an iterator to its last element after sorting (or **begin(range)** if it is empty).

> [!NOTE]
> If the real number of buckets is bigger than _max_buckets, the algorithm will still work correctly but a little less efficiently, as it will require an additional [inplace merge](#coinplace_merge_splice).

> [!NOTE]
> The algorithm uses additional `_max_buckets * (sizeof(pair<size_t, iterator_t<R>>) + sizeof(T))` bytes of memory on the stack, where T is the minimal unsigned type capable of holding _max_buckets (e.g., `uint8_t` if it is <= 255). If that is too much stack memory, consider using the version that takes an allocator.

---

<sub>Defined in header [&lt;enranged/sorting.hpp&gt;](/include/enranged/sorting.hpp)</sub>
```c++
template <size_t _max_buckets = 32, typename Allocator,
          spliceable_range R, typename EqRel, typename Proj1 = std::identity,
          typename Comp = std::ranges::less, typename Proj2 = std::identity>
  requires(_max_buckets > 0 && splice_sortable_range<R, Comp, Proj2>
           && std::indirect_equivalence_relation
              <EqRel, std::projected<std::ranges::iterator_t<R>, Proj1>>)
constexpr std::pair<size_t, std::ranges::borrowed_iterator_t<R>> bucket_sort_splice
  (Allocator&& alloc, R&& range,
   EqRel rel, Proj1 proj1 = {}, Comp comp = {}, Proj2 proj2 = {});
```
Performs a splice-based version of the bucket sorting algorithm on the given range, using a strict weak order, an equivalence relation that is weakly consistent with it (see above for details) and a custom allocator for additional memory. If the relation is (totally) consistent with the order, then the sorting is stable.

**Template parameters**

* `_max_buckets` is the maximum number of equivalence classes used for the given range
* `EqRel` must be an equivalence relation weakly consistent with Comp (see above)
* `Comp` must be a strict weak order (see above)

**Return value**

The size of the range and an iterator to its last element after sorting (or **begin(range)** if it is empty).

> [!NOTE]
> If the real number of buckets is bigger than _max_buckets, the algorithm will still work correctly but a little less efficiently, as it will require an additional [inplace merge](#coinplace_merge_splice).

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

---

### insertion_sort_splice
<sub>Defined in header [&lt;enranged/sorting.hpp&gt;](/include/enranged/sorting.hpp)</sub>
```c++
template <spliceable_range R, left_limit_of<R> L,
          typename Comp = std::ranges::less, typename Proj = std::identity>
  requires(splice_sortable_range<R, Comp, Proj>)
constexpr std::ranges::borrowed_iterator_t<R> insertion_sort_splice
  (R&& range, L left, size_t count, Comp comp = {}, Proj proj = {});
```
Performs a splice-based version of the stable insertion sorting algorithm on the corange (left, left + count] and returns an iterator to its last element.

**Template parameters**

* `Comp` must be a strict weak order (see above)

**Parameters**

* `left` must be a valid left limit of the given range (i.e., a front sentinel or a dereferenceable iterator)
* `count` must not be greater than the number of elements following `left` in the given range

**Return value**

An iterator to the last element of the sorted corange (or [**after(range, left)**](#after) if count is zero).

> [!NOTE]
> Insertion sort works best on small or almost sorted ranges, otherwise a different algorithm should be chosen.

---

<sub>Defined in header [&lt;enranged/sorting.hpp&gt;](/include/enranged/sorting.hpp)</sub>
```c++
template <spliceable_range R,
          typename Comp = std::ranges::less, typename Proj = std::identity>
  requires(std::ranges::sized_range<R> && splice_sortable_range<R, Comp, Proj>)
constexpr std::ranges::borrowed_iterator_t<R> insertion_sort_splice
  (R&& range, Comp comp = {}, Proj proj = {});
```
Performs a splice-based version of the stable insertion sorting algorithm on the given sized range and returns an iterator to its last element.

**Template parameters**

* `Comp` must be a strict weak order (see above)

**Return value**

An iterator to the last element of the range (or equal to **end(range)** if the range is empty).

> [!NOTE]
> Insertion sort works best on small or almost sorted ranges, otherwise a different algorithm should be chosen.

---

### merge_sort_splice
<sub>Defined in header [&lt;enranged/sorting.hpp&gt;](/include/enranged/sorting.hpp)</sub>
```c++
template <spliceable_range R, left_limit_of<R> L,
          typename Comp = std::ranges::less, typename Proj = std::identity>
  requires(splice_sortable_range<R, Comp, Proj>)
constexpr std::ranges::borrowed_iterator_t<R> merge_sort_splice
  (R&& range, L left, size_t count, Comp comp = {}, Proj proj = {});
```
Performs a cache-friendly splice-based version of the stable merge sorting algorithm on the corange (left, left + count] and returns an iterator to its last element.

**Template parameters**

* `Comp` must be a strict weak order (see above)

**Parameters**

* `left` must be a valid left limit of the given range (i.e., a front sentinel or a dereferenceable iterator)
* `count` must not be greater than the number of elements following `left` in the given range

**Return value**

An iterator to the last element of the sorted corange (or [**after(range, left)**](#after) if count is zero).

---

<sub>Defined in header [&lt;enranged/sorting.hpp&gt;](/include/enranged/sorting.hpp)</sub>
```c++
template <spliceable_range R,
          typename Comp = std::ranges::less, typename Proj = std::identity>
  requires(std::ranges::sized_range<R> && splice_sortable_range<R, Comp, Proj>)
constexpr std::ranges::borrowed_iterator_t<R> merge_sort_splice
  (R&& range, Comp comp = {}, Proj proj = {});
```
Performs a cache-friendly splice-based version of the stable merge sorting algorithm on the given sized range and returns an iterator to its last element.

**Template parameters**

* `Comp` must be a strict weak order (see above)

**Return value**

An iterator to the last element of the range (or equal to **end(range)** if the range is empty).

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

## Details
### spliceable_range
<sub>Defined in header [&lt;enranged/splicing.hpp&gt;](/include/enranged/splicing.hpp)</sub>
```c++
template <typename R>
concept spliceable_range = std::ranges::forward_range<R>
  && (__detail::has_splice<R, R>
      || (__detail::has_before_begin<R> && (__detail::has_splice_after<R, R>
                                            || __detail::has_cosplice<R, R>)));
```
The concept of a range that can be naturally spliced, i.e. its subrange can be cheaply moved to the denoted place in the same range.

Examples of such ranges include `std::list` and `std::forward_list` (and linked lists in general). This property allows for efficient implementations of various algorithms that permute elements (like sorting), without the random_access or even the bidirectional requirements.

Such ranges must either:
* define `splice()` methods with the same signature and semantics as in `std::list` (putting a subrange [begin, end) or a given element before the given position without invalidating any iterators);
* or both:
  * define a `noexcept` method `before_begin()` returning a front sentinel (with the same semantics as in `std::forward_list` but not necessarily of the iterator type);
  * and either:
    * define `splice_after()` methods with the same signature and semantics as in `std::forward_list` (putting a subrange (begin, end) or the element following a given one after the given position without invalidating any iterators);
    * or define `cosplice()` methods with the same signature and semantics as explained [below](#cosplice) (putting a subrange (begin, end] or the element following a given one after the given position without invalidating any iterators).

In the second case, the `splice_after()` or `cosplice()` methods must correctly accept the result of `before_begin()` (whether of same type as an iterator or not) as both the position argument and the left limit argument (or the iterator argument in the single element splicing). Note that `before_begin()` can simply return the [**default_front_sentinel**](#default_front_sentinel) as long as this requirement is met with overloads

For singly linked lists (or similar structures) the `cosplice()` option should be preferred, as it allows for O(1) complexity in the subrange case.

---

### spliceable_with_range
<sub>Defined in header [&lt;enranged/splicing.hpp&gt;](/include/enranged/splicing.hpp)</sub>
```c++
template <typename R1, typename R2>
concept spliceable_with_range =
  std::ranges::forward_range<R1> && std::ranges::forward_range<R2>
  && (__detail::has_splice<R1, R2>
      || (__detail::has_before_begin<R1> && __detail::has_before_begin<R2>
          && (__detail::has_splice_after<R1, R2> || __detail::has_cosplice<R1, R2>)));
```
The concept of a range that can be naturally spliced with a subrange of another range (see [**spliceable_range**](#spliceable_range) for details).

> [!NOTE]
> There may be additional constraints imposed on the ranges being spliced, that are not expressible with the language of concepts. For example `std::list<T>::splice()` requires that both the source and destination lists have equal allocators, i.e. `src.get_allocator() == dst.get_allocator()`. Violating requirements like that one may lead to undefined behaviour.

---

### cosplice
<sub>Defined in header [&lt;enranged/splicing.hpp&gt;](/include/enranged/splicing.hpp)</sub>
```c++
template <std::ranges::forward_range D, std::ranges::forward_range S,
          left_limit_of<D> P, left_limit_of<S> L>
  requires(spliceable_with_range<D, S>)
constexpr void cosplice(D&& dst_range, P pos,
                        S&& src_range, L lt, std::ranges::iterator_t<S> rt);
```
Moves the elements in the corange (lt, rt] of the source range after the specified position in the destination range.

The [**cosplice()**](#cosplice) methods are dual to the regular `splice()` ones (in a sense that the latter move a range [a, b) before pos). They are suitable for both singly and doubly linked lists, but unlike the standard `splice_after()` (as in `std::forward_list`) can have O(1) complexity.

**Parameters**

* `pos` must be a valid left limit of `dst_range` (i.e., a front sentinel or a dereferenceable iterator)
* `lt` must be a valid left limit of `src_range` (i.e., a front sentinel or a dereferenceable iterator)
* `rt` must be a dereferenceable iterator of `src_range`, such that `(lt, rt]` is a valid corange (in particular, `lt != rt`)

> [!NOTE]
> The behaviour is undefined if pos is in (lt, rt] or is equal to lt (when dst_range and src_range are the same).

---

<sub>Defined in header [&lt;enranged/splicing.hpp&gt;](/include/enranged/splicing.hpp)</sub>
```c++
template <std::ranges::forward_range D, std::ranges::forward_range S,
          left_limit_of<D> P, left_limit_of<S> I>
  requires(spliceable_with_range<D, S>)
constexpr void cosplice(D&& dst_range, P pos, S&& src_range, I it);
```
Moves the element immediately following the one pointed to by _it_ in the source range after the specified position in the destination range.

**Parameters**

* `pos` must be a valid left limit of `dst_range` (i.e., a front sentinel or a dereferenceable iterator)
* `it` must be a valid left limit of `src_range` (i.e., a front sentinel or a dereferenceable iterator), such that [**after(src_range, it)**](#after) is dereferenceable

> [!NOTE]
> The behaviour is undefined if pos is equal to _it_ or to [**after(src_range, it)**](#after) (when dst_range and src_range are the same).

---

<sub>Defined in header [&lt;enranged/splicing.hpp&gt;](/include/enranged/splicing.hpp)</sub>
```c++
template <spliceable_range R, left_limit_of<R> P, left_limit_of<R> L>
constexpr void cosplice(R&& range, P pos, L lt, std::ranges::iterator_t<R> rt);
```
Moves the elements in the corange (lt, rt] after the specified position in the given range.

**Parameters**

* `pos` must be a valid left limit of the given range (i.e., a front sentinel or a dereferenceable iterator)
* `lt` must be a valid left limit of the given range (i.e., a front sentinel or a dereferenceable iterator)
* `rt` must be a dereferenceable iterator of the given range, such that `(lt, rt]` is a valid corange (in particular, `lt != rt`)

> [!NOTE]
> The behaviour is undefined if pos is in (lt, rt] or is equal to lt.

---

<sub>Defined in header [&lt;enranged/splicing.hpp&gt;](/include/enranged/splicing.hpp)</sub>
```c++
template <spliceable_range R, left_limit_of<R> P, left_limit_of<R> I>
constexpr void cosplice(R&& range, P pos, I it);
```
Moves the element immediately following the one pointed to by _it_ after the specified position in the given range.

**Parameters**

* `pos` must be a valid left limit of the given range (i.e., a front sentinel or a dereferenceable iterator)
* `it` must be a valid left limit of the given range (i.e., a front sentinel or a dereferenceable iterator), such that [**after(range, it)**](#after) is dereferenceable

> [!NOTE]
> The behaviour is undefined if pos is equal to _it_ or to [**after(range, it)**](#after).

---

<sub>Defined in header [&lt;enranged/splicing.hpp&gt;](/include/enranged/splicing.hpp)</sub>
```c++
template <std::ranges::forward_range D, forward_corange S, left_limit_of<D> P>
  requires(spliceable_with_range<D, S>)
constexpr void cosplice(D&& dst_range, P pos, S&& src_range);
```
Moves the elements of the source corange after the specified position in the destination range.

**Parameters**

* `pos` must be a valid left limit of `dst_range` (i.e., a front sentinel or a dereferenceable iterator)
* `src_range` must be a non-empty [forward corange](#forward_corange)

> [!NOTE]
> The behaviour is undefined if pos or [**after(dst_range, pos)**](#after) is in src_range.

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

## Details
### corange
<sub>Defined in header [&lt;enranged/limits.hpp&gt;](/include/enranged/limits.hpp)</sub>
```c++
template <typename R>
concept corange = std::ranges::range<R>
  && (__detail::has_last<R>
      || (std::ranges::bidirectional_range<R> && std::ranges::common_range<R>)
      || (std::ranges::random_access_range<R> && std::ranges::sized_range<R>));
```
The concept of a range with the known last element.

A regular STL range r is of form [a, b) where a=**begin(r)** is an iterator and b=**end(r)** is a sentinel. In contrast, a corange c is of form (x, y] where x=[**before_begin(c)**](#before_begin) is a front sentinel and y=[**last(c)**](#last) is an iterator. In this sense, coranges are dual to ranges.

A range r is considered a corange if it either:
* defines method `r.last()` returning an iterator to the last element of r, that satisfies the following semantic requirements:
  * if r is not empty then `r.last()` must return a dereferenceable iterator;
  * if r is a **forward_range** and is not empty then `next(last(r)) == end(r)` must be true.
* or is both **bidirectional_range** and **common_range**;
* or is both **random_access_range** and **sized_range**.

Unlike with regular ranges, we consider a corange to be valid only if its [**last()**](#last) iterator is dereferenceable. Thus only non-empty coranges can be valid. Functions that accept a corange (either directly with a type constraint, or semantically as a pair (a, b] of a front sentinel + iterator) normally require its validity.

---

### forward_corange
<sub>Defined in header [&lt;enranged/limits.hpp&gt;](/include/enranged/limits.hpp)</sub>
```c++
template <typename R>
concept forward_corange = corange<R> && std::ranges::forward_range<R>;
```
The concept of a corange (i.e., a range with a known last element) that is also a **forward_range**.

> [!NOTE]
> There is additional semantic requirements implied by the equality preserving properties of forward ranges, namely:
> * **begin(r)** == **[after](#after)(r, [before_begin(r)](#before_begin))**;
> * **next([last(r)](#last)**) == **end(r)**.
>
> Both have already been described above and are mentioned here for convenience only.

---

### left_limit_of
<sub>Defined in header [&lt;enranged/limits.hpp&gt;](/include/enranged/limits.hpp)</sub>
```c++
template <typename T, typename R>
concept left_limit_of = std::ranges::range<R>
  && (std::same_as<std::remove_cvref_t<T>, front_sentinel_t<R>>
      || std::same_as<std::remove_cvref_t<T>, std::ranges::iterator_t<R>>);
```
The concept of a type that can serve as a lower bound for a subrange of the given range (i.e. a dereferenceable iterator or a front sentinel).

> [!NOTE]
> An additional semantic requirement here implies that a left limit of a range never compares equal to its **end()**.

---

### right_limit_of
<sub>Defined in header [&lt;enranged/limits.hpp&gt;](/include/enranged/limits.hpp)</sub>
```c++
template <typename T, typename R>
concept right_limit_of = std::ranges::range<R>
  && (std::same_as<std::remove_cvref_t<T>, std::ranges::sentinel_t<R>>
      || std::same_as<std::remove_cvref_t<T>, std::ranges::iterator_t<R>>);
```
The concept of a type that can serve as an upper bound for a subrange of the given range (i.e. a dereferenceable iterator or a sentinel).

> [!NOTE]
> An additional semantic requirement here implies that a right limit of a range never compares equal to its [**before_begin()**](#before_begin).

---

### front_sentinel_t
<sub>Defined in header [&lt;enranged/limits.hpp&gt;](/include/enranged/limits.hpp)</sub>
```c++
template <std::ranges::range R>
using front_sentinel_t = decltype(before_begin(std::declval<R&>()));
```
The sentinel type that preceeds the beginning of the given range (see [**before_begin()**](#before_begin) for details).

> [!NOTE]
> This type is always `std::sentinel_for<std::ranges::iterator_t<R>>`.

---

### default_front_sentinel_t
<sub>Defined in header [&lt;enranged/limits.hpp&gt;](/include/enranged/limits.hpp)</sub>
```c++
struct default_front_sentinel_t: std::unreachable_sentinel_t {};
```
The default implementation of the front sentinel for ranges that do not define a [**before_begin()**](#before_begin) method.

---

### default_front_sentinel
<sub>Defined in header [&lt;enranged/limits.hpp&gt;](/include/enranged/limits.hpp)</sub>
```c++
inline constexpr default_front_sentinel_t default_front_sentinel{};
```
A (global constant) object of type [**default_front_sentinel_t**](#default_front_sentinel_t) that may serve as a universal (unreachable) front sentinel.

---

### after
<sub>Defined in header [&lt;enranged/limits.hpp&gt;](/include/enranged/limits.hpp)</sub>
```c++
template <std::ranges::range R, left_limit_of<R> L>
constexpr std::ranges::iterator_t<R> after(R&& range, L it);
```
Returns an iterator to an element immediately following the given left limit in the given range.

---

### before_begin
<sub>Defined in header [&lt;enranged/limits.hpp&gt;](/include/enranged/limits.hpp)</sub>
```c++
template <std::ranges::range R>
constexpr std::sentinel_for<std::ranges::iterator_t<R>> auto before_begin(R&& range) noexcept;
```
Returns a front sentinel that precedes the beginning of the given range.

This extends the idea of a "reverse end" to ranges that cannot be naturally reversed (e.g., ranges that are not bidirectional or have an unreachable **end()** sentinel), but can still somehow use the concept of a pseudo-iterator pointing before the beggining (e.g., as in `std::forward_list<T>::insert_after()`).

If the range implements a noexcept method `before_begin()` that returns an `std::sentinel_for` its iterator, then the result of that method will be used. Otherwise, [**default_front_sentinel**](#default_front_sentinel) is returned.

A front sentinel s of a range r has the following semantic requirements (that should be kept in mind when implementing `r.before_begin()`):
* s cannot compare equal to any dereferenceable iterator of r or to an iterator that equals the **end(r)**. In particular, in case of a **common_range**, `s == end(r)` must always return false;
* if s has the type of an iterator of r, then its increment must be either dereferenceable or equal to **end(r)**. Furthermore, if r is a **forward_range**, then `++s == begin(r)` must be true;
* s should not be dereferenced and, if r is not **common_range**, does not have to be equality comparable with **end(r)**.

---

### last
<sub>Defined in header [&lt;enranged/limits.hpp&gt;](/include/enranged/limits.hpp)</sub>
```c++
template <corange R>
constexpr std::ranges::iterator_t<R> last(R&& range);
```
Returns an iterator to the last element of the given corange.

> [!NOTE]
> The behaviour is undefined if the range is empty (see above about the validity of coranges).
