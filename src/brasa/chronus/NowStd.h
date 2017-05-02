#pragma once

#include "brasa/chronus/Constants.h"
#include <chrono>
#include <cstdint>

namespace brasa {
namespace chronus {
inline uint64_t NanoNow() {
    return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

inline uint64_t MicroNow() {
    return std::chrono::high_resolution_clock::now().time_since_epoch().count() / NSECS_PER_USEC;
}

inline uint64_t MilliNow() {
    return std::chrono::high_resolution_clock::now().time_since_epoch().count() / NSECS_PER_MSEC;
}
}
}
