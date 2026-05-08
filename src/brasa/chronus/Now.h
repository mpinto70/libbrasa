#pragma once

#include <chrono>
#include <cstdint>

namespace brasa::chronus {

namespace detail {
/**
 * Returns the current instant as a tick count in the given `DURATION_TYPE` unit.
 *
 * Uses `std::chrono::steady_clock`, which is a monotonic clock guaranteed never to go
 * backwards. The returned value is the number of ticks elapsed since the clock's epoch
 * (implementation-defined; typically system boot time).
 *
 * @tparam DURATION_TYPE A `std::chrono` duration type (e.g. `std::chrono::nanoseconds`).
 * @return Tick count since the `steady_clock` epoch in `DURATION_TYPE` units.
 */
template <typename DURATION_TYPE>
inline uint64_t std_now() {
    using std::chrono::duration_cast;
    using std::chrono::steady_clock;
    return duration_cast<DURATION_TYPE>(steady_clock::now().time_since_epoch()).count();
}
} // namespace detail

/**
 * Returns the current instant in nanoseconds since the `steady_clock` epoch.
 */
inline uint64_t nano_now() {
    return ::brasa::chronus::detail::std_now<std::chrono::nanoseconds>();
}

/**
 * Returns the current instant in microseconds since the `steady_clock` epoch.
 */
inline uint64_t micro_now() {
    return ::brasa::chronus::detail::std_now<std::chrono::microseconds>();
}

/**
 * Returns the current instant in milliseconds since the `steady_clock` epoch.
 */
inline uint64_t milli_now() {
    return ::brasa::chronus::detail::std_now<std::chrono::milliseconds>();
}

} // namespace brasa::chronus
