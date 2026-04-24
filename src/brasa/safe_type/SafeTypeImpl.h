/**
 * Core implementation of the safe-type wrappers exposed by `SafeType.h`.
 *
 * The templates in this header preserve the exact layout of the wrapped type while
 * using a tag type and an operation category to prevent accidental mixing of logically
 * different values that share the same representation.
 */

// inspired by https://github.com/rollbear/strong_type
#pragma once

#include <algorithm>
#include <cstring>

namespace brasa::safe_type {
namespace impl {
/// Operation sets available for a wrapped value.
enum class Category {
    Trivial, ///< Enables only identity comparison (`==` and `!=`).
    Ordered, ///< Adds ordering (`<`, `>`, `<=` and `>=`) to `Trivial`.
    Scalar,  ///< Adds arithmetic (`+`, `-`, `*`, `/`, `++`, `--`) to `Ordered`.
};
} // namespace impl

/**
 * Strongly typed wrapper around a trivially copyable underlying value.
 *
 * `SafeType` keeps the same storage layout as `T`, but becomes a distinct C++ type by
 * pairing the value with a tag type. Two wrappers with the same underlying type but
 * different tags are therefore not interchangeable.
 *
 * This primary template implements the categories that only need storage plus comparison:
 * `Trivial` and `Ordered`. The `Scalar` category is specialized below to add mutating
 * arithmetic operators.
 *
 * @tparam T Underlying storage type.
 * @tparam Tag Unique tag type that differentiates one safe type from another.
 * @tparam category Operation category that controls which free operators are available.
 */
template <typename T, typename Tag, impl::Category category>
struct SafeType final {
    /// Type stored by the wrapper.
    using underlying_type = T;
    static_assert(std::is_trivially_copyable_v<T>);
    static_assert(std::is_trivially_constructible_v<T>);

    /// Wrapped value. The field is intentionally public to preserve aggregate semantics.
    T value;

    /**
     * Creates a wrapper from an explicitly converted underlying value.
     *
     * This helper is mainly useful when the caller already needs to cast to
     * `underlying_type` and aggregate initialization would become verbose.
     */
    static constexpr SafeType to_safe(underlying_type v) { return SafeType{ v }; }
};

/**
 * `SafeType` specialization for scalar values.
 *
 * Besides the operations inherited from `Ordered`, this specialization adds the mutating
 * operators required by the free arithmetic operators declared later in the file.
 */
template <typename T, typename Tag>
struct SafeType<T, Tag, impl::Category::Scalar> final {
    /// Type stored by the wrapper.
    using underlying_type = T;
    static_assert(std::is_trivially_copyable_v<T>);
    static_assert(std::is_trivially_constructible_v<T>);

    /// Wrapped value. The field is intentionally public to preserve aggregate semantics.
    T value;

    /// Creates a wrapper from an explicitly converted underlying value.
    static constexpr SafeType to_safe(underlying_type v) { return SafeType{ v }; }

    /// Prefix increment of the wrapped value.
    SafeType& operator++() noexcept(noexcept(++value)) {
        ++value;
        return *this;
    }

    /// Postfix increment of the wrapped value.
    SafeType operator++(int) noexcept(noexcept(value++)) {
        auto tmp = *this;
        value++;
        return tmp;
    }

    /// Prefix decrement of the wrapped value.
    SafeType& operator--() noexcept(noexcept(--value)) {
        --value;
        return *this;
    }

    /// Postfix decrement of the wrapped value.
    SafeType operator--(int) noexcept(noexcept(value--)) {
        auto tmp = *this;
        value--;
        return tmp;
    }

    /// Adds another value of the same safe type.
    SafeType& operator+=(const SafeType& y) noexcept(noexcept(value += y.value)) {
        value += y.value;
        return *this;
    }

    /// Subtracts another value of the same safe type.
    SafeType& operator-=(const SafeType& y) noexcept(noexcept(value -= y.value)) {
        value -= y.value;
        return *this;
    }

    /// Multiplies by a raw underlying value.
    SafeType& operator*=(const T& y) noexcept(noexcept(value *= y)) {
        value *= y;
        return *this;
    }

    /// Divides by a raw underlying value.
    SafeType& operator/=(const T& y) noexcept(noexcept(value /= y)) {
        value /= y;
        return *this;
    }

    /// Unary minus
    SafeType operator-() const noexcept(noexcept(-value)) { return SafeType{ -value }; }

    /// Unary plus
    SafeType operator+() const noexcept(noexcept(+value)) { return SafeType{ +value }; }
};

// ==================================================
// IDENTITY: enabled to all categories
// ==================================================

template <typename T, typename Tag, impl::Category category>
constexpr bool operator==(
      const SafeType<T, Tag, category>& x,
      const SafeType<T, Tag, category>& y) noexcept(noexcept(x.value == y.value)) {
    return x.value == y.value;
}

template <typename T, typename Tag, impl::Category category, size_t N>
constexpr bool operator==(
      const SafeType<T[N], Tag, category>& x,
      const SafeType<T[N], Tag, category>& y) noexcept(noexcept(x.value[0] == y.value[0])) {
    return std::equal(x.value, x.value + N, y.value);
}

template <typename T, typename Tag, impl::Category category>
constexpr bool operator!=(
      const SafeType<T, Tag, category>& x,
      const SafeType<T, Tag, category>& y) noexcept(noexcept(x == y)) {
    return not(x == y);
}

// ==================================================
// SORTING: enabled to categories `Ordered` and `Scalar`
// ==================================================

template <typename T, typename Tag, impl::Category category>
constexpr bool operator<(
      const SafeType<T, Tag, category>& x,
      const SafeType<T, Tag, category>& y) noexcept(noexcept(x.value < y.value))
    requires(category == impl::Category::Ordered || category == impl::Category::Scalar)
{
    return x.value < y.value;
}

template <typename T, typename Tag, impl::Category category, size_t N>
constexpr bool operator<(
      const SafeType<T[N], Tag, category>& x,
      const SafeType<T[N], Tag, category>& y) noexcept(noexcept(x.value[0] < y.value[0]))
    requires(category == impl::Category::Ordered || category == impl::Category::Scalar)
{
    return std::lexicographical_compare(x.value, x.value + N, y.value, y.value + N);
}

template <typename T, typename Tag, impl::Category category>
constexpr bool operator>(
      const SafeType<T, Tag, category>& x,
      const SafeType<T, Tag, category>& y) noexcept(noexcept(x < y))
    requires(category == impl::Category::Ordered || category == impl::Category::Scalar)
{
    return y < x;
}

template <typename T, typename Tag, impl::Category category>
constexpr bool operator<=(
      const SafeType<T, Tag, category>& x,
      const SafeType<T, Tag, category>& y) noexcept(noexcept(x < y))
    requires(category == impl::Category::Ordered || category == impl::Category::Scalar)
{
    return not(y < x);
}

template <typename T, typename Tag, impl::Category category>
constexpr bool operator>=(
      const SafeType<T, Tag, category>& x,
      const SafeType<T, Tag, category>& y) noexcept(noexcept(x < y))
    requires(category == impl::Category::Ordered || category == impl::Category::Scalar)
{
    return not(x < y);
}

// ==================================================
// ARITHMETIC: enabled to category `Scalar`
// ==================================================

template <typename T, typename Tag, impl::Category category>
constexpr SafeType<T, Tag, category> operator+(
      const SafeType<T, Tag, category>& x,
      const SafeType<T, Tag, category>& y) noexcept(noexcept(x.value + y.value))
    requires(category == impl::Category::Scalar)
{
    const T tmp = x.value + y.value;
    return SafeType<T, Tag, category>{ tmp };
}

template <typename T, typename Tag, impl::Category category>
constexpr SafeType<T, Tag, category> operator-(
      const SafeType<T, Tag, category>& x,
      const SafeType<T, Tag, category>& y) noexcept(noexcept(x.value - y.value))
    requires(category == impl::Category::Scalar)
{
    const T tmp = x.value - y.value;
    return SafeType<T, Tag, category>{ tmp };
}

template <typename T, typename Tag, impl::Category category>
constexpr SafeType<T, Tag, category> operator*(
      const SafeType<T, Tag, category>& x,
      typename SafeType<T, Tag, category>::underlying_type y) noexcept(noexcept(x.value * y))
    requires(category == impl::Category::Scalar)
{
    const T tmp = x.value * y;
    return SafeType<T, Tag, category>{ tmp };
}

template <typename T, typename Tag, impl::Category category>
constexpr SafeType<T, Tag, category> operator*(
      typename SafeType<T, Tag, category>::underlying_type x,
      const SafeType<T, Tag, category>& y) noexcept(noexcept(x * y.value))
    requires(category == impl::Category::Scalar)
{
    const T tmp = x * y.value;
    return SafeType<T, Tag, category>{ tmp };
}

template <typename T, typename Tag, impl::Category category>
constexpr SafeType<T, Tag, category> operator/(
      const SafeType<T, Tag, category>& x,
      typename SafeType<T, Tag, category>::underlying_type y) noexcept(noexcept(x.value / y))
    requires(category == impl::Category::Scalar)
{
    const T tmp = x.value / y;
    return SafeType<T, Tag, category>{ tmp };
}

} // namespace brasa::safe_type
