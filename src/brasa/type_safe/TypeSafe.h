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

    base_type& operator+=(const base_type& rh) noexcept(noexcept(value += rh.value)) {
        value += rh.value;
        return *this;
    }
    base_type& operator-=(const base_type& rh) noexcept(noexcept(value -= rh.value)) {
        value -= rh.value;
        return *this;
    }
    base_type& operator*=(const T& rh) noexcept(noexcept(value *= rh)) {
        value *= rh;
        return *this;
    }
    base_type& operator/=(const T& rh) noexcept(noexcept(value /= rh)) {
        value /= rh;
        return *this;
    }
};

/// comparison: enabled to all categories
template <typename T, typename Tag, impl::Category category>
constexpr bool operator==(
      const base_type<T, Tag, category>& lh,
      const base_type<T, Tag, category>& rh) noexcept(noexcept(lh.value == rh.value)) {
    return lh.value == rh.value;
}

/// comparison: enabled to all categories
template <typename T, typename Tag, impl::Category category>
constexpr bool operator!=(
      const base_type<T, Tag, category>& lh,
      const base_type<T, Tag, category>& rh) noexcept(noexcept(lh.value == rh.value)) {
    return not(lh == rh);
}

/// comparison: enabled to all categories (specialization for arrays)
template <typename T, typename Tag, impl::Category category, size_t N>
constexpr bool operator==(
      const base_type<T[N], Tag, category>& lh,
      const base_type<T[N], Tag, category>& rh) noexcept(noexcept(lh.value[0] == rh.value[0])) {
    for (size_t i = 0; i < N; ++i) {
        if (lh.value[i] != rh.value[i])
            return false;
    }
    return true;
}

/// sorting: enabled to categories `Ordered` and `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr
      typename std::enable_if<category == impl::Category::Ordered || category == impl::Category::Scalar, bool>::type
      operator<(
            const base_type<T, Tag, category>& lh,
            const base_type<T, Tag, category>& rh) noexcept(noexcept(lh.value < rh.value)) {
    return lh.value < rh.value;
}

/// sorting: enabled to categories `Ordered` and `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr bool operator>(
      const base_type<T, Tag, category>& lh,
      const base_type<T, Tag, category>& rh) noexcept(noexcept(lh.value < rh.value)) {
    return rh < lh;
}

/// sorting: enabled to categories `Ordered` and `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr bool operator<=(
      const base_type<T, Tag, category>& lh,
      const base_type<T, Tag, category>& rh) noexcept(noexcept(lh.value < rh.value)) {
    return not(rh < lh);
}

/// sorting: enabled to categories `Ordered` and `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr bool operator>=(
      const base_type<T, Tag, category>& lh,
      const base_type<T, Tag, category>& rh) noexcept(noexcept(lh.value < rh.value)) {
    return not(lh < rh);
}

/// arithmetic: enabled to category `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr
      typename std::enable_if<category == impl::Category::Scalar, base_type<T, Tag, category>>::type
      operator+(
            const base_type<T, Tag, category>& lh,
            const base_type<T, Tag, category>& rh) noexcept(noexcept(lh.value + rh.value)) {
    const T tmp = lh.value + rh.value;
    return base_type<T, Tag, category>{ tmp };
}

/// arithmetic: enabled to category `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr
      typename std::enable_if<category == impl::Category::Scalar, base_type<T, Tag, category>>::type
      operator-(
            const base_type<T, Tag, category>& lh,
            const base_type<T, Tag, category>& rh) noexcept(noexcept(lh.value - rh.value)) {
    const T tmp = lh.value - rh.value;
    return base_type<T, Tag, category>{ tmp };
}

/// arithmetic: enabled to category `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr
      typename std::enable_if<category == impl::Category::Scalar, base_type<T, Tag, category>>::type
      operator*(
            const base_type<T, Tag, category>& lh,
            const T& rh) noexcept(noexcept(lh.value* rh)) {
    const T tmp = lh.value * rh;
    return base_type<T, Tag, category>{ tmp };
}

/// arithmetic: enabled to category `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr
      typename std::enable_if<category == impl::Category::Scalar, base_type<T, Tag, category>>::type
      operator*(
            const T& lh,
            const base_type<T, Tag, category>& rh) noexcept(noexcept(lh* rh.value)) {
    const T tmp = lh * rh.value;
    return base_type<T, Tag, category>{ tmp };
}

/// arithmetic: enabled to category `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr
      typename std::enable_if<category == impl::Category::Scalar, base_type<T, Tag, category>>::type
      operator/(
            const base_type<T, Tag, category>& lh,
            const T& rh) noexcept(noexcept(lh.value / rh)) {
    const T tmp = lh.value / rh;
    return base_type<T, Tag, category>{ tmp };
}

// Important definitions. You should use one of the below types
// trivial: [ == and != ]
// ordered: trivial + [ < > <= >= ]
// scalar: ordered + [ + += - -= * (value) / (value) ]
template <typename T, typename Tag>
using trivial = base_type<T, Tag, impl::Category::Trivial>;
template <typename T, typename Tag>
using ordered = base_type<T, Tag, impl::Category::Ordered>;
template <typename T, typename Tag>
using scalar = base_type<T, Tag, impl::Category::Scalar>;

/// cast from underlying to wrapped type
template <typename TYPE>
constexpr TYPE type_cast(typename TYPE::underlying_type value) noexcept {
    return TYPE{ value };
}

template <typename TYPE>
constexpr const typename TYPE::underlying_type& value_cast(const TYPE& t) noexcept {
    return t.value;
}
}
}
