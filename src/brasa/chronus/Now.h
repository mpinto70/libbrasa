#pragma once

#include <brasa/chronus/Constants.h>

#include <chrono>
#include <cstdint>

namespace brasa {
namespace chronus {

template <typename DURATION_TYPE>
inline uint64_t std_now() {
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;
    return duration_cast<DURATION_TYPE>(high_resolution_clock::now().time_since_epoch()).count();
}

/// returns current instant in nanoseconds
inline uint64_t nano_now() {
    return std_now<std::chrono::nanoseconds>();
}

/// returns current instant in microseconds
inline uint64_t micro_now() {
    return std_now<std::chrono::microseconds>();
}

/// returns current instant in milliseconds
inline uint64_t milli_now() {
    return std_now<std::chrono::milliseconds>();
}

} // namespace chronus
} // namespace brasa
