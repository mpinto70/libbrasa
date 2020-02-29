#pragma once

#include <brasa/chronus/Constants.h>

#include <cstdint>
#include <ctime>

namespace brasa {
namespace chronus {

template <clockid_t CLOCK_ID>
struct NowClock {
    uint64_t operator()() const {
        timespec now;
        ::clock_gettime(CLOCK_ID, &now);
        return now.tv_sec * NSECS_PER_SEC + now.tv_nsec;
    }
};
}
}
