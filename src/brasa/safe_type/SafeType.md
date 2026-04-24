# SafeType Library Documentation

- [SafeType Library Documentation](#safetype-library-documentation)
  - [Overview](#overview)
  - [Core Concepts](#core-concepts)
    - [1. The Problem](#1-the-problem)
    - [2. The Solution](#2-the-solution)
  - [Architecture](#architecture)
    - [1. `SafeTypeImpl.h` - Core Implementation](#1-safetypeimplh---core-implementation)
    - [2. `SafeType.h` - Public Interface](#2-safetypeh---public-interface)
    - [3. `SafeTypeHash.h` - Standard Library Integration](#3-safetypehashh---standard-library-integration)
  - [Operation Categories](#operation-categories)
    - [1. Trivial Category](#1-trivial-category)
    - [2. Ordered Category](#2-ordered-category)
    - [3. Scalar Category](#3-scalar-category)
  - [Key Features](#key-features)
    - [1. Zero-Cost Abstraction](#1-zero-cost-abstraction)
    - [2. Aggregate Semantics](#2-aggregate-semantics)
    - [3. Compile-Time Operations](#3-compile-time-operations)
    - [4. Exception Safety](#4-exception-safety)
  - [Usage Patterns](#usage-patterns)
    - [1. Basic Type Definition](#1-basic-type-definition)
    - [2. Conversion Utilities](#2-conversion-utilities)
    - [3. Container Usage](#3-container-usage)
    - [4. Array Support](#4-array-support)
  - [Best Practices](#best-practices)
    - [1. Choose Appropriate Categories](#1-choose-appropriate-categories)
    - [2. Type Conversion Guidelines](#2-type-conversion-guidelines)
  - [Integration with Standard Library](#integration-with-standard-library)
    - [1. Hash Support](#1-hash-support)
    - [2. Algorithms](#2-algorithms)
  - [Performance Characteristics](#performance-characteristics)
  - [Limitations and Considerations](#limitations-and-considerations)
    - [1. No Implicit Conversions](#1-no-implicit-conversions)
    - [2. Underlying Type Requirements](#2-underlying-type-requirements)
    - [3. Array Limitations](#3-array-limitations)
  - [Troubleshooting](#troubleshooting)
    - [1. Common Compilation Errors](#1-common-compilation-errors)
  - [Examples](#examples)
  - [Related Documentation](#related-documentation)

## Overview

The `SafeType` library provides strongly-typed wrappers around primitive types
to prevent accidental mixing of logically different values that share the same
underlying representation. This is particularly useful for domain modeling where
you want to distinguish between different kinds of identifiers, measurements, or
quantities at compile time.

## Core Concepts

### 1. The Problem

Consider this common programming error:

```cpp
void process_order(int user_id, int product_id, int quantity) {
    // Implementation
}

// Later in code...
int user = 123;
int product = 456;
int qty = 2;

// Oops! Arguments are in wrong order - compiler can't catch this
process_order(product, user, qty);  // Runtime bug!
```

### 2. The Solution

SafeType creates distinct types from the same underlying representation:

```cpp
using UserId = brasa::safe_type::Trivial<int, struct UserIdTag>;
using ProductId = brasa::safe_type::Trivial<int, struct ProductIdTag>;
using Quantity = brasa::safe_type::Scalar<int, struct QuantityTag>;

void process_order(UserId user_id, ProductId product_id, Quantity quantity) {
    // Implementation
}

// Later in code...
UserId user{123};
ProductId product{456};
Quantity qty{2};

// Compiler error! Types don't match
// process_order(product, user, qty);  // Won't compile

// Correct usage
process_order(user, product, qty);  // Compiles and runs correctly
```

## Architecture

The library consists of three main components:

### 1. [`SafeTypeImpl.h`](SafeTypeImpl.h) - Core Implementation

**Purpose**: Contains the core `SafeType` template and all operator
implementations.

**Key Components**:

- `SafeType<T, Tag, Category>` - Primary template class
- `impl::Category` enum - Defines operation categories
- Operator overloads for comparison, arithmetic, and ordering
- Template specialization for `Scalar` category

### 2. [`SafeType.h`](SafeType.h) - Public Interface

**Purpose**: Provides convenient type aliases that hide implementation details.

**Key Components**:

- `Trivial<T, Tag>` - Identity comparison only
- `Ordered<T, Tag>` - Identity + ordering comparisons
- `Scalar<T, Tag>` - Identity + ordering + arithmetic operations

### 3. [`SafeTypeHash.h`](SafeTypeHash.h) - Standard Library Integration

**Purpose**: Enables SafeType usage in hash-based standard library containers.

**Key Components**:

- `std::hash` specialization for all SafeType variants
- Zero-overhead hash delegation to underlying type

## Operation Categories

The library provides three levels of operations through the `impl::Category`
enum:

### 1. Trivial Category

**Operations**: Identity comparison (`==`, `!=`)

**Use Cases**:

- Unique identifiers (user IDs, session tokens)
- Enumeration-like values
- Any value where only identity matters

**Example**:

```cpp
using SessionId = brasa::safe_type::Trivial<std::string, struct SessionIdTag>;

SessionId session1{"abc123"};
SessionId session2{"def456"};

// Supported operations
bool same = (session1 == session2);      // false
bool different = (session1 != session2); // true

// Not allowed - no ordering
// bool less = (session1 < session2);    // Compilation error
```

### 2. Ordered Category

**Operations**: Identity comparison + ordering (`<`, `>`, `<=`, `>=`)

**Use Cases**:

- Timestamps, dates
- Priority levels
- Version numbers
- Any value with natural ordering but no arithmetic

**Example**:

```cpp
using Timestamp = brasa::safe_type::Ordered<uint64_t, struct TimestampTag>;

Timestamp t1{1000};
Timestamp t2{2000};

// Supported operations
bool equal = (t1 == t2);      // false
bool less = (t1 < t2);        // true
bool greater_eq = (t1 >= t2); // false

// Not allowed - no arithmetic
// auto sum = t1 + t2;         // Compilation error
```

### 3. Scalar Category

**Operations**: Identity + ordering + arithmetic (`+`, `-`, `*`, `/`, `++`,
`--`, `+=`, `-=`, `*=`, `/=`)

**Use Cases**:

- Quantities (weight, distance, count)
- Monetary amounts
- Physical measurements
- Any value supporting mathematical operations

**Example**:

```cpp
using Distance = brasa::safe_type::Scalar<double, struct DistanceTag>;
using Weight = brasa::safe_type::Scalar<double, struct WeightTag>;

Distance d1{10.5};
Distance d2{5.0};

// All operations supported
bool equal = (d1 == d2);          // false
bool less = (d1 < d2);            // false
Distance sum = d1 + d2;           // 15.5
Distance scaled = d1 * 2.0;       // 21.0
d1 += d2;                         // d1 becomes 15.5

// Type safety preserved
Weight w{100.0};
// auto invalid = d1 + w;         // Compilation error - different types!
```

## Key Features

### 1. Zero-Cost Abstraction

SafeType maintains the exact same memory layout as the underlying type:

```cpp
using UserId = brasa::safe_type::Trivial<int, struct UserIdTag>;

static_assert(sizeof(UserId) == sizeof(int));  // True
static_assert(alignof(UserId) == alignof(int)); // True

// Can be used in packed structures
struct __attribute__((packed)) Message {
    uint8_t type;
    UserId sender;    // No padding added
    uint32_t data;
};
```

### 2. Aggregate Semantics

SafeType preserves aggregate initialization:

```cpp
using Point2D = brasa::safe_type::Trivial<int[2], struct Point2DTag>;

Point2D origin{0, 0};           // Aggregate initialization
Point2D point{{10, 20}};        // Also valid
```

### 3. Compile-Time Operations

Many operations are `constexpr` and work at compile time:

```cpp
using Count = brasa::safe_type::Scalar<int, struct CountTag>;

constexpr Count c1{10};
constexpr Count c2{5};
constexpr Count sum = c1 + c2;   // Computed at compile time
static_assert(sum.value == 15);  // Verified at compile time
```

### 4. Exception Safety

All operations preserve the exception safety of the underlying type:

```cpp
// If T's operations are noexcept, SafeType's operations are too
using SafeInt = brasa::safe_type::Scalar<int, struct SafeIntTag>;
static_assert(noexcept(SafeInt{1} + SafeInt{2}));  // True for int
```

## Usage Patterns

### 1. Basic Type Definition

```cpp
// Step 1: Define tag types (empty structs)
struct UserIdTag {};
struct ProductIdTag {};
struct PriceTag {};

// Step 2: Create type aliases
using UserId = brasa::safe_type::Trivial<int, UserIdTag>;
using ProductId = brasa::safe_type::Trivial<int, ProductIdTag>;
using Price = brasa::safe_type::Scalar<double, PriceTag>;

// Step 3: Use in functions
void add_to_cart(UserId user, ProductId product, Price price) {
    // Type-safe implementation
}
```

### 2. Conversion Utilities

```cpp
using Temperature = brasa::safe_type::Scalar<double, struct TempTag>;

// From underlying type
double raw_temp = 23.5;
Temperature temp = Temperature::to_safe(raw_temp);

// To underlying type
double back_to_raw = temp.value;

// Aggregate initialization (preferred)
Temperature temp2{23.5};
```

### 3. Container Usage

```cpp
#include <brasa/safe_type/SafeTypeHash.h>  // Required for hash containers

using UserId = brasa::safe_type::Trivial<int, struct UserIdTag>;

// Hash-based containers
std::unordered_set<UserId> active_users;
std::unordered_map<UserId, std::string> user_names;

// Sequence containers
std::vector<UserId> user_list;
std::set<UserId> sorted_users;  // Requires Ordered or Scalar category
```

### 4. Array Support

SafeType supports arrays as underlying types:

```cpp
using MacAddress = brasa::safe_type::Trivial<uint8_t[6], struct MacAddressTag>;

MacAddress mac{{0x00, 0x1B, 0x44, 0x11, 0x3A, 0x0C}};

// Comparison works element-wise
MacAddress other{{0x00, 0x1B, 0x44, 0x11, 0x3A, 0x0C}};
bool same = (mac == other);  // true

// Lexicographical ordering for Ordered/Scalar categories
using SerialNumber = brasa::safe_type::Ordered<char[10], struct SerialTag>;
SerialNumber sn1{{"ABC1234567"}};
SerialNumber sn2{{"ABC1234568"}};
bool ordered = (sn1 < sn2);  // true
```

## Best Practices

### 1. Choose Appropriate Categories

```cpp
// ? Too restrictive - timestamps should be orderable
using Timestamp = brasa::safe_type::Trivial<uint64_t, struct TimestampTag>;

// ? Appropriate category
using Timestamp = brasa::safe_type::Ordered<uint64_t, struct TimestampTag>;

// ? Too permissive - IDs shouldn't have arithmetic
using UserId = brasa::safe_type::Scalar<int, struct UserIdTag>;

// ? Appropriate category
using UserId = brasa::safe_type::Trivial<int, struct UserIdTag>;
```

### 2. Type Conversion Guidelines

```cpp
// ? Explicit conversion when needed
int raw_id = get_raw_id();
CustomerId customer = CustomerId::to_safe(raw_id);

// ? Direct construction when values are known
process_customer(CustomerId{12345});

// ? Avoid when types should not be interchangeable
// (No implicit conversions - this is by design!)
```

## Integration with Standard Library

### 1. Hash Support

```cpp
#include <brasa/safe_type/SafeTypeHash.h>

using Key = brasa::safe_type::Trivial<std::string, struct KeyTag>;

std::unordered_map<Key, int> map;
std::unordered_set<Key> set;

Key k{"hello"};
map[k] = 42;
set.insert(k);
```

### 2. Algorithms

SafeType works with standard algorithms:

```cpp
using Score = brasa::safe_type::Ordered<int, struct ScoreTag>;

std::vector<Score> scores{{100}, {85}, {92}, {78}};

// Sorting works due to comparison operators
std::sort(scores.begin(), scores.end());

// Finding works due to equality operator
auto it = std::find(scores.begin(), scores.end(), Score{92});
```

## Performance Characteristics

| Operation           | Time Complexity    | Space Complexity   | Notes                             |
| ------------------- | ------------------ | ------------------ | --------------------------------- |
| Construction        | O(1)               | O(1)               | Same as underlying type           |
| Comparison          | O(1) or O(n)       | O(1)               | O(n) for arrays, O(1) for scalars |
| Arithmetic          | O(1)               | O(1)               | Same as underlying type           |
| Hash                | O(1) or O(n)       | O(1)               | Depends on underlying type        |
| Container insertion | Same as underlying | Same as underlying | No additional overhead            |

## Limitations and Considerations

### 1. No Implicit Conversions

```cpp
using Distance = brasa::safe_type::Scalar<double, struct DistanceTag>;
using Weight = brasa::safe_type::Scalar<double, struct WeightTag>;

Distance d{10.0};
Weight w{5.0};

// ? This will not compile - different types
// auto result = d + w;

// ? Explicit conversion if intentional
auto result = Distance{d.value + w.value};  // Manual conversion
```

### 2. Underlying Type Requirements

```cpp
// ? Good - trivially copyable types
using UserId = brasa::safe_type::Trivial<int, struct UserIdTag>;
using Name = brasa::safe_type::Trivial<std::array<char, 50>, struct NameTag>;

// ? Won't work - std::string is not trivially copyable
// using Message = brasa::safe_type::Trivial<std::string, struct MessageTag>;
```

### 3. Array Limitations

```cpp
// Array support is more limited
using Buffer = brasa::safe_type::Trivial<char[100], struct BufferTag>;

Buffer b{};
// ? Comparison works
bool equal = (b == Buffer{});

// ? Some operations may not be available for all array types
// Check compiler errors for specific limitations
```

## Troubleshooting

### 1. Common Compilation Errors

**Error**: "static assertion failed: trivially copyable"

```cpp
// ? Problem: Non-trivially copyable type
using SafeString = brasa::safe_type::Trivial<std::string, struct Tag>;

// ? Solution: Use trivially copyable types
using SafeStringView = brasa::safe_type::Trivial<std::string_view, struct Tag>;
```

**Error**: "no matching function for call to operator+"

```cpp
using Id = brasa::safe_type::Trivial<int, struct IdTag>;
Id id1{1}, id2{2};
// ? Problem: Trivial category doesn't support arithmetic
// auto sum = id1 + id2;

// ? Solution: Use Scalar category if arithmetic is needed
using Counter = brasa::safe_type::Scalar<int, struct CounterTag>;
```

**Error**: "no matching function for call to std::hash"

```cpp
#include <brasa/safe_type/SafeType.h>
using Key = brasa::safe_type::Trivial<int, struct KeyTag>;
// ? Problem: Missing hash header
// std::unordered_map<Key, int> map;

// ? Solution: Include hash support
#include <brasa/safe_type/SafeTypeHash.h>
std::unordered_map<Key, int> map;  // Now works
```

## Examples

See the [test files](../../../test/brasa/safe_type/) for comprehensive usage
examples and edge cases.

## Related Documentation

- [`SafeTypeHash.md`](SafeTypeHash.md) - Hash function documentation
- [`SafeTypeImpl.h`](SafeTypeImpl.h) - Core implementation details
- [`SafeType.h`](SafeType.h) - Public interface
- [`SafeTypeHash.h`](SafeTypeHash.h) - Hash support implementation