#pragma once
// based in work from Alex Stepanov https://www.youtube.com/watch?v=aIHAEYyoTUc&list=PLHxtyCq_WDLXryyw91lahwdtpZsmo4BGD

#include <utility>

namespace brasa {
namespace instrument {

template <typename T>
class Singleton {
public:
    T value;
    using value_type = T;

    // conversions
    explicit constexpr Singleton(const T& x) : value(x) {}
    explicit constexpr Singleton(T&& x) : value(std::move(x)) {}
    explicit operator T() const { return value; }

    // semi-regular operations
    constexpr Singleton(const Singleton& x) : value(x.value) {}
    constexpr Singleton(Singleton&& x) : value(std::move(x.value)) {}
    constexpr Singleton() = default;
    ~Singleton() = default;
    Singleton& operator=(const Singleton& rhs) {
        value = rhs.value;
        return *this;
    }

    // regular operations
    friend
    constexpr bool operator == (const Singleton& lhs, const Singleton& rhs) {
        return lhs.value == rhs.value;
    }
    friend
    constexpr bool operator != (const Singleton& lhs, const Singleton& rhs) {
        return not(lhs == rhs);
    }

    // totally ordered operations
    friend
    constexpr bool operator < (const Singleton& lhs, const Singleton& rhs) {
        return lhs.value < rhs.value;
    }
    friend
    constexpr bool operator > (const Singleton& lhs, const Singleton& rhs) {
        return rhs < lhs;
    }
    friend
    constexpr bool operator <= (const Singleton& lhs, const Singleton& rhs) {
        return not(rhs < lhs);
    }
    friend
    constexpr bool operator >= (const Singleton& lhs, const Singleton& rhs) {
        return not(lhs < rhs);
    }
};
}
}
