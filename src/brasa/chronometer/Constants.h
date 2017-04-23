#pragma once
#include <cstdint>

namespace brasa {
namespace chronometer {

constexpr uint32_t MSECS_PER_SEC = 1000;
constexpr uint32_t USECS_PER_SEC = 1000 * MSECS_PER_SEC;
constexpr uint32_t NSECS_PER_SEC = 1000 * USECS_PER_SEC;
constexpr uint32_t USECS_PER_MSEC = 1000;
constexpr uint32_t NSECS_PER_MSEC = 1000 * USECS_PER_MSEC;
constexpr uint32_t NSECS_PER_USEC = 1000;

}
}
