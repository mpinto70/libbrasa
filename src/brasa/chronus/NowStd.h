#pragma once

#include "brasa/chronus/Constants.h"
#include <chrono>
#include <cstdint>

namespace brasa {
namespace chronus {

template <typename DURATION_TYPE>
inline uint64_t std_now() {
    using namespace std::chrono;
    return duration_cast<DURATION_TYPE>(high_resolution_clock::now().time_since_epoch()).count();
}

inline uint64_t nano_now() {
    return std_now<std::chrono::nanoseconds>();
}

inline uint64_t micro_now() {
    return std_now<std::chrono::microseconds>();
}

inline uint64_t milli_now() {
    return std_now<std::chrono::milliseconds>();
}
}
}
