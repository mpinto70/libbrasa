#pragma once

#include <cstdint>
#include <thread>
#include <chrono>

namespace brasa {
namespace chronus {

template <typename DURATION_TYPE>
inline void StdSleep(const uint32_t sleep_length) {
    const DURATION_TYPE dur(sleep_length);
    std::this_thread::sleep_for(dur);
}

inline void NanoSleep(const uint32_t sleep_length) {
    return StdSleep<std::chrono::nanoseconds>(sleep_length);
}

inline void MicroSleep(const uint32_t sleep_length) {
    return StdSleep<std::chrono::microseconds>(sleep_length);
}

inline void MilliSleep(const uint32_t sleep_length) {
    return StdSleep<std::chrono::milliseconds>(sleep_length);
}
}
}
