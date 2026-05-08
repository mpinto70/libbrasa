#pragma once

#include <chrono>
#include <cstdint>
#include <thread>

namespace brasa::chronus {

namespace detail {
/**
 * Suspends the current thread for `sleep_length` ticks in the given `DURATION_TYPE` unit.
 *
 * Delegates to `std::this_thread::sleep_for`, so the actual sleep duration may be longer
 * than requested due to OS scheduling.
 *
 * @tparam DURATION_TYPE A `std::chrono` duration type (e.g. `std::chrono::milliseconds`).
 * @param sleep_length   Number of ticks to sleep, expressed in `DURATION_TYPE` units.
 */
template <typename DURATION_TYPE>
inline void std_sleep(const uint32_t sleep_length) {
    const DURATION_TYPE dur(sleep_length);
    std::this_thread::sleep_for(dur);
}
} // namespace detail

/**
 * Suspends the current thread for `sleep_length` nanoseconds.
 *
 * @param sleep_length Number of nanoseconds to sleep.
 */
inline void nano_sleep(const uint32_t sleep_length) {
    ::brasa::chronus::detail::std_sleep<std::chrono::nanoseconds>(sleep_length);
}

/**
 * Suspends the current thread for `sleep_length` microseconds.
 *
 * @param sleep_length Number of microseconds to sleep.
 */
inline void micro_sleep(const uint32_t sleep_length) {
    ::brasa::chronus::detail::std_sleep<std::chrono::microseconds>(sleep_length);
}

/**
 * Suspends the current thread for `sleep_length` milliseconds.
 *
 * @param sleep_length Number of milliseconds to sleep.
 */
inline void milli_sleep(const uint32_t sleep_length) {
    ::brasa::chronus::detail::std_sleep<std::chrono::milliseconds>(sleep_length);
}
} // namespace brasa::chronus
