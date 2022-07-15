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
struct safe_type {
    using underlying_type = T;
    T value;
    // This function is useful when a value to use to initialize the safe type needs a static_cast,
    // because TYPE{value} wouldn´t compile. In these cases, you would have to write
    // TYPE{static_cast<underlying_type>(value)}. This funcion make it a little less verbose:
    // TYPE.to_safe(value)
    static constexpr safe_type to_safe(underlying_type v) { return safe_type{ v }; }
};

/// A wrapper that adds the ability to make arithmetic operations
template <typename T, typename Tag>
struct safe_type<T, Tag, impl::Category::Scalar> {
    using underlying_type = T;
    T value;
    static constexpr safe_type to_safe(underlying_type v) { return safe_type{ v }; }

    safe_type& operator++() noexcept(noexcept(++value)) {
        ++value;
        return *this;
    }

    safe_type operator++(int) noexcept(noexcept(value++)) {
        auto tmp = *this;
        value++;
        return tmp;
    }

    safe_type& operator--() noexcept(noexcept(--value)) {
        --value;
        return *this;
    }

    safe_type operator--(int) noexcept(noexcept(value--)) {
        auto tmp = *this;
        value--;
        return tmp;
    }

    safe_type& operator+=(const safe_type& y) noexcept(noexcept(value += y.value)) {
        value += y.value;
        return *this;
    }
    safe_type& operator-=(const safe_type& y) noexcept(noexcept(value -= y.value)) {
        value -= y.value;
        return *this;
    }
    safe_type& operator*=(const T& y) noexcept(noexcept(value *= y)) {
        value *= y;
        return *this;
    }
    safe_type& operator/=(const T& y) noexcept(noexcept(value /= y)) {
        value /= y;
        return *this;
    }
};

/// comparison: enabled to all categories
template <typename T, typename Tag, impl::Category category>
constexpr bool operator==(
      const safe_type<T, Tag, category>& x,
      const safe_type<T, Tag, category>& y) noexcept(noexcept(x.value == y.value)) {
    return x.value == y.value;
}

/// comparison: enabled to all categories
template <typename T, typename Tag, impl::Category category>
constexpr bool operator!=(
      const safe_type<T, Tag, category>& x,
      const safe_type<T, Tag, category>& y) noexcept(noexcept(x.value == y.value)) {
    return not(x == y);
}

/// comparison: enabled to all categories (specialization for arrays)
template <typename T, typename Tag, impl::Category category, size_t N>
constexpr bool operator==(
      const safe_type<T[N], Tag, category>& x,
      const safe_type<T[N], Tag, category>& y) noexcept(noexcept(x.value[0] == y.value[0])) {
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
            const safe_type<T, Tag, category>& x,
            const safe_type<T, Tag, category>& y) noexcept(noexcept(x.value < y.value)) {
    return x.value < y.value;
}

/// sorting: enabled to categories `Ordered` and `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr bool operator>(
      const safe_type<T, Tag, category>& x,
      const safe_type<T, Tag, category>& y) noexcept(noexcept(x.value < y.value)) {
    return y < x;
}

/// sorting: enabled to categories `Ordered` and `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr bool operator<=(
      const safe_type<T, Tag, category>& x,
      const safe_type<T, Tag, category>& y) noexcept(noexcept(x.value < y.value)) {
    return not(y < x);
}

/// sorting: enabled to categories `Ordered` and `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr bool operator>=(
      const safe_type<T, Tag, category>& x,
      const safe_type<T, Tag, category>& y) noexcept(noexcept(x.value < y.value)) {
    return not(x < y);
}

/// arithmetic: enabled to category `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr
      typename std::enable_if<category == impl::Category::Scalar, safe_type<T, Tag, category>>::type
      operator+(
            const safe_type<T, Tag, category>& x,
            const safe_type<T, Tag, category>& y) noexcept(noexcept(x.value + y.value)) {
    const T tmp = x.value + y.value;
    return safe_type<T, Tag, category>{ tmp };
}

/// arithmetic: enabled to category `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr
      typename std::enable_if<category == impl::Category::Scalar, safe_type<T, Tag, category>>::type
      operator-(
            const safe_type<T, Tag, category>& x,
            const safe_type<T, Tag, category>& y) noexcept(noexcept(x.value - y.value)) {
    const T tmp = x.value - y.value;
    return safe_type<T, Tag, category>{ tmp };
}

/// arithmetic: enabled to category `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr
      typename std::enable_if<category == impl::Category::Scalar, safe_type<T, Tag, category>>::type
      operator*(
            const safe_type<T, Tag, category>& x,
            typename safe_type<T, Tag, category>::underlying_type
                  y) noexcept(noexcept(x.value* y)) {
    const T tmp = x.value * y;
    return safe_type<T, Tag, category>{ tmp };
}

/// arithmetic: enabled to category `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr
      typename std::enable_if<category == impl::Category::Scalar, safe_type<T, Tag, category>>::type
      operator*(
            typename safe_type<T, Tag, category>::underlying_type x,
            const safe_type<T, Tag, category>& y) noexcept(noexcept(x* y.value)) {
    const T tmp = x * y.value;
    return safe_type<T, Tag, category>{ tmp };
}

/// arithmetic: enabled to category `Scalar`
template <typename T, typename Tag, impl::Category category>
constexpr
      typename std::enable_if<category == impl::Category::Scalar, safe_type<T, Tag, category>>::type
      operator/(
            const safe_type<T, Tag, category>& x,
            typename safe_type<T, Tag, category>::underlying_type
                  y) noexcept(noexcept(x.value / y)) {
    const T tmp = x.value / y;
    return safe_type<T, Tag, category>{ tmp };
}
}
}
