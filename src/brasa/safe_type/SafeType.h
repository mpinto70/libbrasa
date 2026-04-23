/**
 * Public entry point for the safe-type aliases.
 *
 * Prefer the aliases declared here over instantiating `SafeType` directly. Each alias picks
 * the operation category that matches the semantics of the wrapped value while still using
 * an explicit tag type to prevent accidental interchange with other values.
 */

// inspired by https://github.com/rollbear/strong_type
#pragma once

#include <brasa/safe_type/SafeTypeImpl.h>

#include <cstring>

namespace brasa::safe_type {
/**
 * Safe type with identity comparison only.
 *
 * Use this alias when values should be assignable and comparable for equality, but there is
 * no meaningful ordering or arithmetic between them.
 *
 * @tparam T Underlying storage type.
 * @tparam Tag Unique tag type that makes the wrapper distinct.
 */
template <typename T, typename Tag>
using Trivial = SafeType<T, Tag, impl::Category::Trivial>;

/**
 * Safe type with identity and ordering comparisons.
 *
 * Use this alias when values support equality and sorting semantics, but arithmetic would not
 * make sense for the domain being modeled.
 *
 * @tparam T Underlying storage type.
 * @tparam Tag Unique tag type that makes the wrapper distinct.
 */
template <typename T, typename Tag>
using Ordered = SafeType<T, Tag, impl::Category::Ordered>;

/**
 * Safe type with identity, ordering, and arithmetic operations.
 *
 * Use this alias when values represent a scalar quantity that can be added to or subtracted
 * from the same type, and multiplied or divided by the raw underlying type.
 *
 * @tparam T Underlying storage type.
 * @tparam Tag Unique tag type that makes the wrapper distinct.
 */
template <typename T, typename Tag>
using Scalar = SafeType<T, Tag, impl::Category::Scalar>;
} // namespace brasa::safe_type
