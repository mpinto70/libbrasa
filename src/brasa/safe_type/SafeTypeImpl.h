// inspired by https://github.com/rollbear/strong_type
#pragma once

#include <algorithm>
#include <cstring>

namespace brasa::safe_type {
namespace impl {
/// Categories of wrapped values
enum class Category {
    Trivial, ///< only identity (== and !=)
    Ordered, ///< ordering (<, >, <= and >=)
    Scalar,  ///< arithmetic (+, -, * and /)
};
} // namespace impl

/** A wrapper to make POD types different types.
 * `T`: is the underlying type
 * `Tag`: is a type to make the wrapper unique
 * `Category`: is used to enable/disable operations
 */
template <typename T, typename Tag, impl::Category category>
struct SafeType final {
    using underlying_type = T;
    T value;
    // This function is useful when a value to use to initialize the safe type needs a static_cast,
    // because TYPE{value} wouldn't compile. In these cases, you would have to write
    // TYPE{static_cast<underlying_type>(value)}. This function make it a little less verbose:
    // TYPE.to_safe(value)
    static constexpr SafeType to_safe(underlying_type v) { return SafeType{ v }; }
};

/// A wrapper that adds the ability to make arithmetic operations
template <typename T, typename Tag>
struct SafeType<T, Tag, impl::Category::Scalar> final {
    using underlying_type = T;
    T value;
    static constexpr SafeType to_safe(underlying_type v) { return SafeType{ v }; }

    SafeType& operator++() noexcept(noexcept(++value)) {
        ++value;
        return *this;
    }

    SafeType operator++(int) noexcept(noexcept(value++)) {
        auto tmp = *this;
        value++;
        return tmp;
    }

    SafeType& operator--() noexcept(noexcept(--value)) {
        --value;
        return *this;
    }

    SafeType operator--(int) noexcept(noexcept(value--)) {
        auto tmp = *this;
        value--;
        return tmp;
    }

    SafeType& operator+=(const SafeType& y) noexcept(noexcept(value += y.value)) {
        value += y.value;
        return *this;
    }

    SafeType& operator-=(const SafeType& y) noexcept(noexcept(value -= y.value)) {
        value -= y.value;
        return *this;
    }

    SafeType& operator*=(const T& y) noexcept(noexcept(value *= y)) {
        value *= y;
        return *this;
    }

    SafeType& operator/=(const T& y) noexcept(noexcept(value /= y)) {
        value /= y;
        return *this;
    }
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
