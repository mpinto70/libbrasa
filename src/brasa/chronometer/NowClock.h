#pragma once

#include "brasa/chronometer/Constants.h"
#include <cstdint>
#include <ctime>

namespace chronometer {

template <clockid_t CLOCK_ID>
struct NowClock {
    uint64_t operator()() const {
        static timespec now;
        ::clock_gettime(CLOCK_ID, &now);
        return now.tv_sec * NSECS_PER_SEC + now.tv_nsec;
    }
};

}
