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
    explicit Singleton(const T& x) : value(x) {}
    explicit Singleton(T&& x) : value(std::move(x)) {}
    explicit operator T() const { return value; }

    // semi-regular operations
    Singleton(const Singleton& x) : value(x.value) {}
    Singleton(Singleton&& x) : value(std::move(x.value)) {}
    Singleton() = default;
    ~Singleton() = default;
    Singleton& operator=(const Singleton& rhs) {
        value = rhs.value;
        return *this;
    }

    // regular operations
    friend
    bool operator == (const Singleton& lhs, const Singleton& rhs) {
        return lhs.value == rhs.value;
    }
    friend
    bool operator != (const Singleton& lhs, const Singleton& rhs) {
        return not(lhs.value == rhs.value);
    }

    // totally ordered operations
    friend
    bool operator < (const Singleton& lhs, const Singleton& rhs) {
        return lhs.value < rhs.value;
    }
    friend
    bool operator > (const Singleton& lhs, const Singleton& rhs) {
        return rhs < lhs;
    }
    friend
    bool operator <= (const Singleton& lhs, const Singleton& rhs) {
        return not(rhs < lhs);
    }
    friend
    bool operator >= (const Singleton& lhs, const Singleton& rhs) {
        return not(lhs < rhs);
    }
};
}
}
