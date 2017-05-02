#pragma once

#include "brasa/chronus/Constants.h"
#include <chrono>
#include <cstdint>

namespace brasa {
namespace chronus {

template <typename DURATION_TYPE>
inline uint64_t StdNow() {
    using namespace std::chrono;
    return duration_cast<DURATION_TYPE>(high_resolution_clock::now().time_since_epoch()).count();
}

inline uint64_t NanoNow() {
    return StdNow<std::chrono::nanoseconds>();
}

inline uint64_t MicroNow() {
    return StdNow<std::chrono::microseconds>();
}

inline uint64_t MilliNow() {
    return StdNow<std::chrono::milliseconds>();
}
}
}
