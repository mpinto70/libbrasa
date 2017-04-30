#pragma once

#include "brasa/chronus/Constants.h"

#include <algorithm>
#include <cstdint>
#include <ctime>

namespace brasa {
namespace chronus {

template<typename NOW_FUNC, typename SLEEPER>
class Waiter {
public:
    Waiter(NOW_FUNC&& now, uint32_t units_of_time, SLEEPER&& sleeper)
        : now_(std::move(now)),
          units_of_time_(units_of_time),
          sleep_(std::move(sleeper)),
          sleep_size_(units_of_time / 10 + 1),
          end_of_time_(now_() + units_of_time_) {
    }
    Waiter(Waiter&&) = default;
    Waiter& operator =(Waiter&&) = default;
    bool elapsed() const {
        return now_() >= end_of_time_;
    }
    void wait() const {
        while (!elapsed()) {
            sleep_(sleep_size_);
        }
    }
    void reset() {
        end_of_time_ = now_() + units_of_time_;
    }
private:
    const NOW_FUNC now_;
    const uint32_t units_of_time_;
    const SLEEPER sleep_;
    const uint32_t sleep_size_;
    uint64_t end_of_time_;

    Waiter(const Waiter&) = delete;
    Waiter& operator =(const Waiter&) = delete;
};

template<typename NOW_FUNC, typename SLEEPER>
Waiter<NOW_FUNC, SLEEPER> make_waiter(NOW_FUNC&& func, uint32_t units_of_time, SLEEPER&& sleeper) {
    return Waiter<NOW_FUNC, SLEEPER>(std::forward<NOW_FUNC>(func), units_of_time, std::forward<SLEEPER>(sleeper));
}

}
}
