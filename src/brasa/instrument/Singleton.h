#pragma once

#include <utility>

namespace brasa::instrument {

/**
 * A value-semantic wrapper around a single value of type `T`.
 *
 * Based on work by Alex Stepanov:
 * https://www.youtube.com/watch?v=aIHAEYyoTUc&list=PLHxtyCq_WDLXryyw91lahwdtpZsmo4BGD
 *
 * Note: despite the name, this is NOT the Singleton design pattern. It is a
 * minimally-structured container for a single value that models the concepts of
 * semi-regular, regular, and totally ordered types, depending on the properties of `T`.
 *
 * @tparam T The wrapped value type. Must support the operations used (copy, move,
 *           comparison) according to the concept being modelled.
 */
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
    constexpr Singleton(const Singleton& x) = default;
    constexpr Singleton(Singleton&& x) noexcept = default;
    Singleton& operator=(const Singleton& y) noexcept = default;
    Singleton& operator=(Singleton&& x) noexcept = default;

    // regular and totally ordered operations
    friend bool operator==(const Singleton& x, const Singleton& y)
        requires std::equality_comparable<T>
    = default;
    friend auto operator<=>(const Singleton& x, const Singleton& y)
        requires std::three_way_comparable<T>
    = default;
};
} // namespace brasa::instrument
