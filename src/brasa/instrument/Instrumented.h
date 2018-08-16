#pragma once
// based in work from Alex Stepanov https://www.youtube.com/watch?v=aIHAEYyoTUc&list=PLHxtyCq_WDLXryyw91lahwdtpZsmo4BGD

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
    explicit constexpr Instrumented(const T& x)
          : value(x) {
        ++counts[conversion_construction];
    }
    explicit constexpr Instrumented(T&& x)
          : value(std::move(x)) {
        ++counts[conversion_move_construction];
    }
    explicit operator T() const {
        ++counts[conversion];
        return value;
    }

    // semi-regular operations
    constexpr Instrumented(const Instrumented& x)
          : value(x.value) {
        ++counts[copy_construction];
    }
    constexpr Instrumented(Instrumented&& x) noexcept
          : value(std::move(x.value)) {
        ++counts[move_construction];
    }
    constexpr Instrumented() noexcept {
        ++counts[default_construction];
    }
    ~Instrumented() {
        ++counts[destruction];
    }
    Instrumented& operator=(const Instrumented& rhs) {
        ++counts[assignment];
        value = rhs.value;
        return *this;
    }

    // regular operations
    friend constexpr bool operator==(const Instrumented& lhs, const Instrumented& rhs) {
        ++counts[equality];
        return lhs.value == rhs.value;
    }
    friend constexpr bool operator!=(const Instrumented& lhs, const Instrumented& rhs) {
        return not(lhs == rhs);
    }

    // totally ordered operations
    friend constexpr bool operator<(const Instrumented& lhs, const Instrumented& rhs) {
        ++counts[comparison];
        return lhs.value < rhs.value;
    }
    friend constexpr bool operator>(const Instrumented& lhs, const Instrumented& rhs) {
        return rhs < lhs;
    }
    friend constexpr bool operator<=(const Instrumented& lhs, const Instrumented& rhs) {
        return not(rhs < lhs);
    }
    friend constexpr bool operator>=(const Instrumented& lhs, const Instrumented& rhs) {
        return not(lhs < rhs);
    }
};
}
}
