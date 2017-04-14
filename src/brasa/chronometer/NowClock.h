#pragma once

#include <ctime>

namespace chronometer {
inline uint64_t NowWallClock() {
    static timespec now;
    ::clock_gettime(CLOCK_REALTIME, &now);
    return now.tv_sec * 1000 * 1000 * 1000 + now.tv_nsec;
}
}
