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
