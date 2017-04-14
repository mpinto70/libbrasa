#pragma once

#include <ctime>

namespace chronometer {

template <clockid_t CLOCK_ID>
struct NowClock {
    uint64_t operator()() const {
        static timespec now;
        ::clock_gettime(CLOCK_ID, &now);
        return now.tv_sec * 1000 * 1000 * 1000 + now.tv_nsec;
    }
};

}
