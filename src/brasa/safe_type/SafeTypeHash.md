# SafeTypeHash.h Documentation

- [SafeTypeHash.h Documentation](#safetypehashh-documentation)
  - [Overview](#overview)
  - [Purpose](#purpose)
  - [Features](#features)
    - [1. Hash Function Specialization](#1-hash-function-specialization)
  - [Usage Examples](#usage-examples)
    - [1. Basic Usage with Standard Containers](#1-basic-usage-with-standard-containers)
    - [2. Hash Consistency](#2-hash-consistency)
  - [Design Decisions](#design-decisions)
    - [1. Hash Value Calculation](#1-hash-value-calculation)
    - [2. Tag and Category Independence](#2-tag-and-category-independence)
    - [3. Exception Safety](#3-exception-safety)
  - [Requirements](#requirements)
    - [1. Type Requirements](#1-type-requirements)
    - [2. Include Dependencies](#2-include-dependencies)
  - [Performance Characteristics](#performance-characteristics)
  - [Integration with Standard Library](#integration-with-standard-library)
  - [Best Practices](#best-practices)
    - [1. Always Include When Using Hash Containers](#1-always-include-when-using-hash-containers)
    - [2. Consider Hash Distribution](#2-consider-hash-distribution)
  - [Related Files](#related-files)
  - [Compatibility](#compatibility)

## Overview

`SafeTypeHash.h` provides hash function support for `SafeType` wrappers,
enabling their use in standard library hash-based containers such as
`std::unordered_map` and `std::unordered_set`.

## Purpose

The `SafeType` library creates distinct types from the same underlying
representation using tag types. However, for hash-based containers to work
correctly, these wrapped types need specialized hash functions. This header
provides that specialization.

## Features

### 1. Hash Function Specialization

```cpp
namespace std {
template <typename T, typename Tag, ::brasa::safe_type::impl::Category category>
struct hash<::brasa::safe_type::SafeType<T, Tag, category>> {
    size_t operator()(const ::brasa::safe_type::SafeType<T, Tag, category>& x) const
        noexcept(noexcept(hash<T>{}(x.value)));
};
}
```

**Key Properties:**

- **Delegation**: Hash computation delegates to the underlying type's hash
  function
- **Type Independence**: Tag type and operation category don't affect hash
  values
- **Performance**: Zero overhead &mdash; same performance as hashing the
  underlying type
- **Exception Safety**: `noexcept` specification matches underlying hash
  function

## Usage Examples

### 1. Basic Usage with Standard Containers

```cpp
#include <brasa/safe_type/SafeType.h>
#include <brasa/safe_type/SafeTypeHash.h>
#include <unordered_map>
#include <unordered_set>

// Define safe types using aliases
using UserId = brasa::safe_type::Trivial<int, struct UserIdTag>;
using ProductId = brasa::safe_type::Ordered<int, struct ProductIdTag>;

// Use in hash-based containers
std::unordered_set<UserId> active_users;
std::unordered_map<ProductId, std::string> product_names;

// Add elements
active_users.insert(UserId{123});
product_names[ProductId{456}] = "Widget";
```

### 2. Hash Consistency

```cpp
UserId user1{42};
UserId user2{42};

// Same hash values for same underlying values
assert(std::hash<UserId>{}(user1) == std::hash<UserId>{}(user2));

// Different SafeTypes with same underlying value have same hash
ProductId product{42};
assert(std::hash<UserId>{}(user1) == std::hash<ProductId>{}(product));
```

## Design Decisions

### 1. Hash Value Calculation

**Decision**: Use the underlying type's hash function directly.

**Rationale**:

- **Simplicity**: Avoids complex hash combination logic
- **Performance**: No overhead compared to hashing the underlying value
- **Consistency**: Same underlying values always produce same hashes
- **Standard Compliance**: Follows expected behavior for wrapper types

**Trade-off**: Different SafeTypes with same underlying value have identical
hashes, which could theoretically lead to more collisions in mixed-type
scenarios.

### 2. Tag and Category Independence

**Decision**: Tag type and operation category don't influence hash computation.

**Rationale**:

- **Logical Consistency**: Hash should depend only on the actual data
- **Performance**: Avoids type-dependent hash computation overhead
- **Interoperability**: Allows comparison of hashes across different SafeType
  instantiations

### 3. Exception Safety

**Decision**: Make hash function conditionally `noexcept` based on underlying type.

**Rationale**:

- **Standards Compliance**: Follows std::hash requirements
- **Performance**: Allows compiler optimizations when underlying hash is
  `noexcept`
- **Safety**: Maintains exception safety guarantees of the underlying type

## Requirements

### 1. Type Requirements

For `SafeType<T, Tag, category>` to be hashable:

1. **T must be hashable**: `std::hash<T>` must be specialized and well-defined
2. **T must be the underlying type**: Not an array type (arrays have separate
   specializations in [`SafeTypeImpl.h`](SafeTypeImpl.h))

### 2. Include Dependencies

```cpp
#include <brasa/safe_type/SafeTypeImpl.h>  // For SafeType definition
#include <functional>                       // For std::hash
```

## Performance Characteristics

- **Time Complexity**: O(1) - same as underlying type's hash function
- **Space Complexity**: O(1) - no additional storage
- **Runtime Overhead**: Zero - direct delegation to underlying hash

## Integration with Standard Library

This specialization enables SafeType instances to work seamlessly with:

- `std::unordered_map<SafeType<T,Tag,Cat>, V>`
- `std::unordered_set<SafeType<T,Tag,Cat>>`
- `std::unordered_multimap<SafeType<T,Tag,Cat>, V>`
- `std::unordered_multiset<SafeType<T,Tag,Cat>>`
- Any other template that requires `std::hash` specialization

## Best Practices

### 1. Always Include When Using Hash Containers

```cpp
// ? Wrong - missing hash support
#include <brasa/safe_type/SafeType.h>
using UserId = brasa::safe_type::Trivial<int, struct UserIdTag>;
std::unordered_map<UserId, std::string> users;  // Compilation error

// ? Correct - includes hash support
#include <brasa/safe_type/SafeType.h>
#include <brasa/safe_type/SafeTypeHash.h>
using UserId = brasa::safe_type::Trivial<int, struct UserIdTag>;
std::unordered_map<UserId, std::string> users;  // Works correctly
```

### 2. Consider Hash Distribution

```cpp
// If using custom underlying types, ensure good hash distribution
struct CustomId {
    uint64_t value;
    bool operator==(const CustomId& other) const { return value == other.value; }
};

// Provide good hash function for underlying type
namespace std {
template<>
struct hash<CustomId> {
    size_t operator()(const CustomId& id) const {
        return std::hash<uint64_t>{}(id.value);
    }
};
}

using SafeCustomId = brasa::safe_type::Trivial<CustomId, struct SafeCustomIdTag>;
// Now SafeCustomId works well in hash containers
```

## Related Files

- [SafeTypeImpl.h](SafeTypeImpl.h): Core SafeType implementation
- [SafeType.h](SafeType.h): Public interface and convenience macros
- Array hash specializations (if any) in SafeTypeImpl.h

## Compatibility

- **C++ Standard**: Requires C++11 or later (for `noexcept`)
- **Compilers**: Compatible with GCC, Clang, MSVC
- **Dependencies**: Standard library only (`<functional>`)