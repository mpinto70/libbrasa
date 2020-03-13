# Type Safe package
This package contains facilities to wrap POD types in safe structures with
limited operations, to prevent wrong assignments and passing the wrong
arguments. This code is inspired in [this
site](https://github.com/rollbear/strong_type).

There are three different categories (`enum class Category`) of types:

|           | assignable         | comparison (`==`, `!=`) | ordering (`<`, `>`, `<=`, `>=`) | arithmetic operations (`+`, `-`, `*`, `/`)<sup>*</sup> |
| :-------- | :----------------: | :---------------------: | :-----------------------------: | :----------------------------------------------------: |
| `Trivial` | :heavy_check_mark: | :heavy_check_mark:      |                                 |                                                        |
| `Ordered` | :heavy_check_mark: | :heavy_check_mark:      | :heavy_check_mark:              |                                                        |
| `Scalar`  | :heavy_check_mark: | :heavy_check_mark:      | :heavy_check_mark:              | :heavy_check_mark:                                     |

<sup>*</sup> `*` and `/` operates on native types only.

The important types defined in this package are:

* `trivial`: a type of category `Category::Trivial`
* `ordered`: a type of category `Category::Ordered`
* `scalar`: a type of category `Category::Scalar`

The wrappers don't add space to the underlying type, so they can be used in
`struct`s where the underlying type would have been. For example, the two
`struct`s below have the same memory representation:

```cpp
struct WithNativeTypes {
    uint32_t price;
    char name[5];
    uint64_t time;
} __attribute__((packed));
using Price = brasa::type_safe::scalar<uint32_t, struct Price_>;
using Name = brasa::type_safe::trivial<char[5], struct Name_>;
using Time = brasa::type_safe::ordered<uint64_t, struct Time_>;
struct WithWrappedTypes {
    Price price;
    Name name;
    Time time;
} __attribute__((packed));

static_assert(sizeof(WithNativeTypes) == sizeof(WithWrappedTypes));
static_assert(offsetof(WithNativeTypes, price) == offsetof(WithWrappedTypes, price));
static_assert(offsetof(WithNativeTypes, name) == offsetof(WithWrappedTypes, name));
static_assert(offsetof(WithNativeTypes, time) == offsetof(WithWrappedTypes, time));

char buffer[sizeof(WithNativeTypes)] = {};

const WithNativeTypes source = { 0x11223344, { 'A', 'B', 'C', 'D', 'E' }, 0x8877665544332211 };
memcpy(buffer, &source, sizeof(source));
Name expected_name = { { 'A', 'B', 'C', 'D', 'E' } };

auto destination = reinterpret_cast<const WithWrappedTypes*>(buffer);
assert(destination->price.value == source.price);
assert(memcmp(destination->name.value, source.name, sizeof(source.name)) == 0);
assert(destination->name == expected_name);
assert(destination->time.value == source.time);
```

## `base_type` class

`base_type` is the only class in this package. It is the wrapper around native
types. It is parameterized by the underlying type (`T`), a tag type (`Tag`) and
the `Category`. The **tag type** is present to prevent the interpretation of a
type as another one at function parameters and operations (see [this
page](https://github.com/rollbear/strong_type) for details).

`base_type` can be used to represent values in categories `Trivial` and
`Ordered`. For types of category `Scalar` there is an overload that adds
operators: `+=` and `-=` (for other `Scalar` types of same `Tag`), and `*=` and
`/=` (for values of underlying type).
