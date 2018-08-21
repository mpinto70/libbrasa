#pragma once

#include "brasa/chronus/Constants.h"

#include <algorithm>
#include <cstdint>
#include <ctime>

namespace brasa {
namespace chronus {

template <typename NOW_FUNC, typename SLEEPER_FUNC>
class Waiter {
public:
    Waiter(NOW_FUNC now, uint32_t units_of_time, SLEEPER_FUNC sleeper)
          : now_(std::move(now)),
            units_of_time_(units_of_time),
            sleep_(std::move(sleeper)),
            sleep_size_(units_of_time / 10 + 1),
            end_of_time_(now_() + units_of_time_) {
    }
    // no copies just moves
    Waiter(const Waiter&) = delete;
    Waiter& operator=(const Waiter&) = delete;
    Waiter(Waiter&&) noexcept = default;
    Waiter& operator=(Waiter&&) noexcept = default;
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
    const SLEEPER_FUNC sleep_;
    const uint32_t sleep_size_;
    uint64_t end_of_time_;
};

template <typename NOW_FUNC, typename SLEEPER_FUNC>
Waiter<NOW_FUNC, SLEEPER_FUNC> make_waiter(NOW_FUNC func,
      const uint32_t units_of_time,
      SLEEPER_FUNC sleeper) {
    return Waiter<NOW_FUNC, SLEEPER_FUNC>(std::forward<NOW_FUNC>(func),
          units_of_time,
          std::forward<SLEEPER_FUNC>(sleeper));
}
}
}
