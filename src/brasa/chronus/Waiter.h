#pragma once

#include <brasa/chronus/Constants.h>

#include <cstdint>
#include <utility>

namespace brasa::chronus {

/**
 * A timer that blocks the calling thread until a specified duration has elapsed.
 *
 * `NOW_FUNC` must be a callable with signature `T()` where `T` is an arithmetic type
 * representing a tick count (e.g. the functions from `Now.h`).  Each call must return
 * a value greater than or equal to the previous call.
 *
 * `SLEEPER_FUNC` must be a callable with signature `void(uint32_t)` that suspends the
 * current thread for the given number of ticks in the same unit as `NOW_FUNC`
 * (e.g. the functions from `SleepStd.h`).  Its `operator()` must be `const`-callable
 * since `wait()` is a `const` member.
 *
 * The internal sleep interval is set to 1/10 of `units_of_time` (minimum 1) to poll
 * the clock without busy-waiting.
 *
 * Waiter objects are move-only (copy is disabled).
 *
 * Prefer constructing instances through the `make_waiter()` factory to benefit from
 * template argument deduction.
 */
template <typename NOW_FUNC, typename SLEEPER_FUNC>
class Waiter {
public:
    /**
     * Tick type returned by `NOW_FUNC`, used to store `end_of_time_`.
     */
    using TimeT = std::invoke_result_t<NOW_FUNC>;

    Waiter(NOW_FUNC&& now, uint32_t units_of_time, SLEEPER_FUNC&& sleeper)
          : now_(std::move(now)),
            units_of_time_(units_of_time),
            sleep_(std::move(sleeper)),
            sleep_size_(units_of_time >= 10 ? units_of_time / 10 : 1),
            end_of_time_(now_() + units_of_time_) {}
    // no copies just moves
    Waiter(const Waiter&) = delete;
    Waiter& operator=(const Waiter&) = delete;
    Waiter(Waiter&&) noexcept = default;
    Waiter& operator=(Waiter&&) noexcept = default;

    /**
     * Returns `true` if the wait period has passed.
     */
    [[nodiscard]] bool elapsed() const { return now_() >= end_of_time_; }

    /**
     * Blocks the calling thread until the wait period has passed.
     *
     * Polls `elapsed()` in a loop, sleeping for `sleep_size_` ticks between checks.
     */
    void wait() const {
        while (!elapsed()) {
            sleep_(sleep_size_);
        }
    }

    /**
     * Resets the wait period, setting the new deadline to `now() + units_of_time`.
     */
    void reset() { end_of_time_ = now_() + units_of_time_; }

private:
    NOW_FUNC now_;           ///< the function that returns the current time
    uint32_t units_of_time_; ///< the total number of time units to wait for
    SLEEPER_FUNC sleep_;     ///< the function that sleeps for a given number of time units
    uint32_t sleep_size_;    ///< units_of_time/10, or 1 if units_of_time is less than 10
    TimeT end_of_time_;      ///< the time point at which the wait period ends
};

/**
 * Factory function that creates a `Waiter` with template argument deduction.
 *
 * @tparam NOW_FUNC      Deduced from `now`; must satisfy the `Waiter` requirements.
 * @tparam SLEEPER_FUNC  Deduced from `sleeper`; must satisfy the `Waiter` requirements.
 * @param now            Callable that returns the current tick count.
 * @param units_of_time  Duration to wait, expressed in the tick unit of `now`.
 * @param sleeper        Callable that suspends the thread for a given number of ticks.
 * @return A `Waiter<NOW_FUNC, SLEEPER_FUNC>` whose deadline is set to `now() + units_of_time`.
 */
template <typename NOW_FUNC, typename SLEEPER_FUNC>
Waiter<NOW_FUNC, SLEEPER_FUNC>
      make_waiter(NOW_FUNC&& now, const uint32_t units_of_time, SLEEPER_FUNC&& sleeper) {
    return Waiter<NOW_FUNC, SLEEPER_FUNC>(
          std::forward<NOW_FUNC>(now),
          units_of_time,
          std::forward<SLEEPER_FUNC>(sleeper));
}
} // namespace brasa::chronus
