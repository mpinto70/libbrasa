#pragma once
// based in work from Alex Stepanov
// https://www.youtube.com/watch?v=aIHAEYyoTUc&list=PLHxtyCq_WDLXryyw91lahwdtpZsmo4BGD this is not a
// Singleton pattern.

#include <utility>

namespace brasa::instrument {

template <typename T>
class Singleton {
public:
    T value;
    using value_type = T;

    // conversions
    explicit constexpr Singleton(T x) : value(std::move(x)) {}
    explicit operator T() const { return value; }

    // semi-regular operations
    constexpr Singleton() = default;
    constexpr Singleton(const Singleton& x) : value(x.value) {}
    constexpr Singleton(Singleton&& x) noexcept = default;
    Singleton& operator=(const Singleton& y) {
        T tmp = y.value;
        value = std::move(tmp);
        return *this;
    }
    Singleton& operator=(Singleton&& x) noexcept = default;

    // regular operations
    friend constexpr bool operator==(const Singleton& x, const Singleton& y) {
        return x.value == y.value;
    }
    friend constexpr bool operator!=(const Singleton& x, const Singleton& y) { return not(x == y); }

    // totally ordered operations
    friend constexpr bool operator<(const Singleton& x, const Singleton& y) {
        return x.value < y.value;
    }
    friend constexpr bool operator>(const Singleton& x, const Singleton& y) { return y < x; }
    friend constexpr bool operator<=(const Singleton& x, const Singleton& y) { return not(y < x); }
    friend constexpr bool operator>=(const Singleton& x, const Singleton& y) { return not(x < y); }
};
} // namespace brasa::instrument
