/**
 * Hash function specialization for SafeType wrappers.
 *
 * This header provides std::hash specialization for SafeType instances, enabling
 * their use in standard library hash-based containers like std::unordered_map
 * and std::unordered_set.
 *
 * The hash implementation delegates to the underlying type's hash function,
 * ensuring that SafeType instances with the same underlying value produce
 * the same hash code regardless of their tag type or operation category.
 */

// inspired by https://github.com/rollbear/strong_type
#pragma once

#include <brasa/safe_type/SafeTypeImpl.h>

#include <functional>

namespace std {
/**
 * Hash function specialization for `brasa::safe_type::SafeType`.
 *
 * Enables `SafeType` instances to be used as keys in hash-based standard library
 * containers. The hash value is computed by delegating to the hash function
 * of the underlying type `T`.
 *
 * @tparam T Underlying storage type that must be hashable
 * @tparam Tag Unique tag type (does not affect hash computation)
 * @tparam category Operation category (does not affect hash computation)
 */
template <typename T, typename Tag, ::brasa::safe_type::impl::Category category>
struct hash<::brasa::safe_type::SafeType<T, Tag, category>> {
    /**
     * Computes hash value for a SafeType instance.
     *
     * @param x SafeType instance to hash
     * @return Hash value computed from the underlying value
     */
    size_t operator()(const ::brasa::safe_type::SafeType<T, Tag, category>& x) const
          noexcept(noexcept(hash<T>{}(x.value))) {
        return hash<T>{}(x.value);
    }
};
} // namespace std
