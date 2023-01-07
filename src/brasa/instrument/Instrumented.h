#pragma once
// based in work from Alex Stepanov
// https://www.youtube.com/watch?v=aIHAEYyoTUc&list=PLHxtyCq_WDLXryyw91lahwdtpZsmo4BGD

#include <cstddef>
#include <utility>

namespace brasa {
namespace instrument {

struct InstrumentedCounter {
    enum operations {
        n,
        destruction,
        default_construction,
        conversion_construction,
        conversion_move_construction,
        copy_construction,
        move_construction,
        conversion,
        assignment,
        equality,
        comparison,
        NUMBER_OPS // this has to be the last
    };
    static size_t counts[NUMBER_OPS];
    static const char* counter_names[NUMBER_OPS];
    static void initialize(size_t m);
};

template <typename T>
class Instrumented : private InstrumentedCounter {
public:
    T value;
    using value_type = T;

    // conversions
    explicit constexpr Instrumented(const T& x) : value(x) { ++counts[conversion_construction]; }
    explicit constexpr Instrumented(T&& x) : value(std::move(x)) {
        ++counts[conversion_move_construction];
    }
    explicit operator T() const {
        ++counts[conversion];
        return value;
    }

    // semi-regular operations
    constexpr Instrumented(const Instrumented& x) : value(x.value) { ++counts[copy_construction]; }
    constexpr Instrumented(Instrumented&& x) noexcept : value(std::move(x.value)) {
        ++counts[move_construction];
    }
    constexpr Instrumented() noexcept { ++counts[default_construction]; }
    ~Instrumented() { ++counts[destruction]; }
    Instrumented& operator=(const Instrumented& y) {
        ++counts[assignment];
        value = y.value;
        return *this;
    }

    // regular operations
    friend constexpr bool operator==(const Instrumented& x, const Instrumented& y) {
        ++counts[equality];
        return x.value == y.value;
    }
    friend constexpr bool operator!=(const Instrumented& x, const Instrumented& y) {
        return not(x == y);
    }

    // totally ordered operations
    friend constexpr bool operator<(const Instrumented& x, const Instrumented& y) {
        ++counts[comparison];
        return x.value < y.value;
    }
    friend constexpr bool operator>(const Instrumented& x, const Instrumented& y) { return y < x; }
    friend constexpr bool operator<=(const Instrumented& x, const Instrumented& y) {
        return not(y < x);
    }
    friend constexpr bool operator>=(const Instrumented& x, const Instrumented& y) {
        return not(x < y);
    }
};
} // namespace instrument
} // namespace brasa
