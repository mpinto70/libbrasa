#pragma once

#include <chrono>
#include <cstdint>
#include <thread>

namespace brasa {
namespace chronus {

template <typename DURATION_TYPE>
inline void std_sleep(const uint32_t sleep_length) {
    const DURATION_TYPE dur(sleep_length);
    std::this_thread::sleep_for(dur);
}

/// suspends the current thread for `sleep_length` nanoseconds
inline void nano_sleep(const uint32_t sleep_length) {
    return std_sleep<std::chrono::nanoseconds>(sleep_length);
}

/// suspends the current thread for `sleep_length` microseconds
inline void micro_sleep(const uint32_t sleep_length) {
    return std_sleep<std::chrono::microseconds>(sleep_length);
}

/// suspends the current thread for `sleep_length` milliseconds
inline void milli_sleep(const uint32_t sleep_length) {
    return std_sleep<std::chrono::milliseconds>(sleep_length);
}
}
}
