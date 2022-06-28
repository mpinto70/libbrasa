# Type Safe package

This package contains facilities to wrap POD types in safe structures with
limited operations, to prevent wrong assignments and passing the wrong
arguments. This code is inspired in [this
site](https://github.com/rollbear/strong_type), and [a book from Alexander
Stepanov and Paul McJones](http://elementsofprogramming.com/).

There are three different categories (see `enum class Category`) of types:

|           |     assignable     | comparison (`==`, `!=`) | ordering (`<`, `>`, `<=`, `>=`) | arithmetic operations (`+`, `-`, `*`, `/`, `++`, `--`)<sup>*</sup> |
| :-------- | :----------------: | :---------------------: | :-----------------------------: | :----------------------------------------------------------------: |
| `Trivial` | :heavy_check_mark: |   :heavy_check_mark:    |                                 |                                                                    |
| `Ordered` | :heavy_check_mark: |   :heavy_check_mark:    |       :heavy_check_mark:        |                                                                    |
| `Scalar`  | :heavy_check_mark: |   :heavy_check_mark:    |       :heavy_check_mark:        |                         :heavy_check_mark:                         |

<sup>*</sup> `*` and `/` operates on native types only.

## Strong typing

The utilities in this package are intended to avoid wrong assignment, wrong
operations, and passing the wrong parameter to functions. Consider you have a
function that receives the product code, and quantity of items to compute
the final price and reduce the inventory, also, suppose that all these values
are represented as `uint32_t` types:

```cpp
uint32_t unsafe_sell(uint32_t product_id, uint32_t quantity);
```

It would be possible to call this function as:

```cpp
uint32_t product_id = 54981;
uint32_t quantity = 35;
uint32_t price = unsafe_sell(product_id, quantity);
```

Note that it would also be possible to call it as

```cpp
uint32_t price = unsafe_sell(quantity, product_id); // not what you wanted
```

On the other hand, if you use the type safe utilities, your code would look like:

```cpp
using ProductId = brasa::type_safe::ordered<uint32_t, struct ProductId_>;
using Price = brasa::type_safe::scalar<uint32_t, struct Price_>;
using Quantity = brasa::type_safe::scalar<uint32_t, struct Quantity_>;

Price safe_sell(ProductId product_id, Quantity quantity);
```

It would be possible to call this function as:

```cpp
ProductId product_id{ 54981 };
Quantity quantity{ 35 };
Price price = safe_sell(product_id, quantity);
```

None of the following lines would compile, thus preventing wrong uses at compile
time:

```cpp
Price price = safe_sell(quantity, product_id); // wrong parameter order
Quantity price = safe_sell(product_id, quantity); // wrong return assignment
```

## Use in `struct`s

It is also possible to embed these wrappers in structures for serialization. The
wrappers don't add space to the underlying type, so they can be used in
`struct`s where the underlying type would have been. For example, the two
`struct`s below have the same memory representation:

```cpp
using ProductId = brasa::type_safe::ordered<uint32_t, struct ProductId_>;
using Name = brasa::type_safe::trivial<char[10], struct Name_>;
using Price = brasa::type_safe::scalar<uint32_t, struct Price_>;

struct unsafe_struct {
    uint32_t product_id;
    char name[10];
    uint32_t price;
} __attribute__((packed));

struct safe_struct {
    ProductId product_id;
    Name name;
    Price price;
} __attribute__((packed));

static_assert(sizeof(unsafe_struct) == sizeof(safe_struct));
static_assert(offsetof(unsafe_struct, product_id) == offsetof(safe_struct, product_id));
static_assert(offsetof(unsafe_struct, name) == offsetof(safe_struct, name));
static_assert(offsetof(unsafe_struct, price) == offsetof(safe_struct, price));
```

## Technical details

The important types defined in this package are:

* `trivial`: a type of category `Category::Trivial`
* `ordered`: a type of category `Category::Ordered`
* `scalar`: a type of category `Category::Scalar`

Below is an example of two different `struct`s used for serialization and
de-serialization. The struct with native types is written to a buffer, that
later is reinterpreted as the struct with the wrapped types and the values
recovered are the same.

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

### `base_type` class

`base_type` is the only class in this package. It is the wrapper around native
types. It is parameterized by the underlying type (`T`), a tag type (`Tag`) and
the `Category`. The **tag type** is present to prevent the interpretation of a
type as another one at function parameters and operations (see [this
page](https://github.com/rollbear/strong_type) for details).

`base_type` can be used to represent values in categories `Trivial` and
`Ordered`. For types of category `Scalar` there is an overload that adds
operators: `+=` and `-=` (for other `Scalar` types of same `Tag`), and `*=` and
`/=` (for values of underlying type).
