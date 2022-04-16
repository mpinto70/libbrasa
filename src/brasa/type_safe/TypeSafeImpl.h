// inspired by https://github.com/rollbear/strong_type
#pragma once

#include <array>
#include <cstring>
#include <type_traits>

namespace brasa {
namespace type_safe {
namespace impl {
/// Categories of wrapped values
enum class Category {
    Trivial, ///< only identity (== and !=)
    Ordered, ///< ordering (<, >, <= and >=)
    Scalar,  ///< arithmetic (+, -, * and /)
};
}

/** A wrapper to make POD types different types.
 * `T`: is the underlying type
 * `Tag`: is a type to make the wrapper unique
 * `Category`: is used to enable/disable operations
 */
template <typename T, typename Tag, impl::Category category>
struct base_type {
    using underlying_type = T;
    T value;
};

/// A wrapper that adds the ability to make arithmetic operations
template <typename T, typename Tag>
struct base_type<T, Tag, impl::Category::Scalar> {
    using underlying_type = T;
    T value;

    base_type& operator+=(const base_type& y) noexcept(noexcept(value += y.value)) {
        value += y.value;
        return *this;
    }
    base_type& operator-=(const base_type& y) noexcept(noexcept(value -= y.value)) {
        value -= y.value;
        return *this;
    }
    base_type& operator*=(const T& y) noexcept(noexcept(value *= y)) {
        value *= y;
        return *this;
    }
    base_type& operator/=(const T& y) noexcept(noexcept(value /= y)) {
        value /= y;
        return *this;
    }
};

/// comparison: enabled to all categories
template <typename T, typename Tag, impl::Category category>
constexpr bool operator==(
      const base_type<T, Tag, category>& x,
      const base_type<T, Tag, category>& y) noexcept(noexcept(x.value == y.value)) {
    return x.value == y.value;
}

/// comparison: enabled to all categories
template <typename T, typename Tag, impl::Category category>
constexpr bool operator!=(
      const base_type<T, Tag, category>& x,
      const base_type<T, Tag, category>& y) noexcept(noexcept(x.value == y.value)) {
    return not(x == y);
}

/// comparison: enabled to all categories (specialization for arrays)
template <typename T, typename Tag, impl::Category category, size_t N>
constexpr bool operator==(
      const base_type<T[N], Tag, category>& x,
      const base_type<T[N], Tag, category>& y) noexcept(noexcept(x.value[0] == y.value[0])) {
    for (size_t i = 0; i < N; ++i) {
        if (x.value[i] != y.value[i])
            return false;
    }
    return true;
}

/// sorting: enabled to categories `Ordered` and `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr typename std::enable_if<
      category == impl::Category::Ordered || category == impl::Category::Scalar,
      bool>::type
      operator<(
            const base_type<T, Tag, category>& x,
            const base_type<T, Tag, category>& y) noexcept(noexcept(x.value < y.value)) {
    return x.value < y.value;
}

/// sorting: enabled to categories `Ordered` and `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr bool operator>(
      const base_type<T, Tag, category>& x,
      const base_type<T, Tag, category>& y) noexcept(noexcept(x.value < y.value)) {
    return y < x;
}

/// sorting: enabled to categories `Ordered` and `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr bool operator<=(
      const base_type<T, Tag, category>& x,
      const base_type<T, Tag, category>& y) noexcept(noexcept(x.value < y.value)) {
    return not(y < x);
}

/// sorting: enabled to categories `Ordered` and `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr bool operator>=(
      const base_type<T, Tag, category>& x,
      const base_type<T, Tag, category>& y) noexcept(noexcept(x.value < y.value)) {
    return not(x < y);
}

/// arithmetic: enabled to category `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr
      typename std::enable_if<category == impl::Category::Scalar, base_type<T, Tag, category>>::type
      operator+(
            const base_type<T, Tag, category>& x,
            const base_type<T, Tag, category>& y) noexcept(noexcept(x.value + y.value)) {
    const T tmp = x.value + y.value;
    return base_type<T, Tag, category>{ tmp };
}

/// arithmetic: enabled to category `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr
      typename std::enable_if<category == impl::Category::Scalar, base_type<T, Tag, category>>::type
      operator-(
            const base_type<T, Tag, category>& x,
            const base_type<T, Tag, category>& y) noexcept(noexcept(x.value - y.value)) {
    const T tmp = x.value - y.value;
    return base_type<T, Tag, category>{ tmp };
}

/// arithmetic: enabled to category `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr
      typename std::enable_if<category == impl::Category::Scalar, base_type<T, Tag, category>>::type
      operator*(
            const base_type<T, Tag, category>& x,
            typename base_type<T, Tag, category>::underlying_type
                  y) noexcept(noexcept(x.value* y)) {
    const T tmp = x.value * y;
    return base_type<T, Tag, category>{ tmp };
}

/// arithmetic: enabled to category `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr
      typename std::enable_if<category == impl::Category::Scalar, base_type<T, Tag, category>>::type
      operator*(
            typename base_type<T, Tag, category>::underlying_type x,
            const base_type<T, Tag, category>& y) noexcept(noexcept(x* y.value)) {
    const T tmp = x * y.value;
    return base_type<T, Tag, category>{ tmp };
}

/// arithmetic: enabled to category `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr
      typename std::enable_if<category == impl::Category::Scalar, base_type<T, Tag, category>>::type
      operator/(
            const base_type<T, Tag, category>& x,
            typename base_type<T, Tag, category>::underlying_type
                  y) noexcept(noexcept(x.value / y)) {
    const T tmp = x.value / y;
    return base_type<T, Tag, category>{ tmp };
}
}
}
