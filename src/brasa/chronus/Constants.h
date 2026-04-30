#pragma once
#include <cstdint>

namespace brasa::chronus {
/** Time unit conversion constants. */
constexpr uint64_t MSECS_PER_SEC = 1000;          ///< milliseconds per second
constexpr uint64_t USECS_PER_SEC = 1'000'000;     ///< microseconds per second
constexpr uint64_t NSECS_PER_SEC = 1'000'000'000; ///< nanoseconds per second
constexpr uint64_t USECS_PER_MSEC = 1000;         ///< microseconds per millisecond
constexpr uint64_t NSECS_PER_MSEC = 1'000'000;    ///< nanoseconds per millisecond
constexpr uint64_t NSECS_PER_USEC = 1000;         ///< nanoseconds per microsecond
} // namespace brasa::chronus
