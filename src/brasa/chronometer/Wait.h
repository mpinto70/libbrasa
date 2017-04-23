#pragma once

#include "brasa/chronometer/Constants.h"

#include <algorithm>
#include <cstdint>
#include <ctime>

namespace chronometer {

template<typename NOW_FUNC>
class Waiter {
public:
    Waiter(NOW_FUNC&& now)
        : now_(std::move(now)) {
    }
    Waiter(Waiter&&) = default;
    Waiter& operator =(Waiter&&) = default;
    void wait(uint64_t nsecs) const {
        constexpr timespec SLEEP = {0, 50 * NSECS_PER_USEC};
        const auto tf = now_() + nsecs;
        while (now_() < tf) {
            ::nanosleep(&SLEEP, nullptr);
        }
    }
private:
    const NOW_FUNC now_;

    Waiter(const Waiter&) = delete;
    Waiter& operator =(const Waiter&) = delete;
};

template<typename NOW_FUNC>
Waiter<NOW_FUNC> make_waiter(NOW_FUNC&& func) {
    return Waiter<NOW_FUNC>(std::forward<NOW_FUNC>(func));
}

}


