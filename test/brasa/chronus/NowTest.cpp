#include <brasa/chronus/Constants.h>
#include <brasa/chronus/Now.h>

#include <gtest/gtest.h>

#include <ctime>

namespace brasa::chronus {
namespace {

// Calls func() 100 times in succession and verifies each call returns a value
// greater than or equal to the previous, confirming the clock is non-decreasing.
template <typename NOW_FUNC>
void verifyNow(NOW_FUNC func) {
    for (unsigned i = 0; i < 100; ++i) {
        const uint64_t t0 = func();
        const uint64_t t1 = func();
        EXPECT_LE(t0, t1) << "iteration " << i;
    }
}

// Sleeps for ~50 ms and verifies that the elapsed time reported by func() is
// at least 50 ms worth of ticks in the given unit (expressed in nanoseconds via
// threshold_ns for the comparison).
template <typename NOW_FUNC>
void verifyUniformity(NOW_FUNC func, uint64_t threshold_ticks) {
    constexpr timespec SLEEP = { 0, 50 * NSECS_PER_MSEC };
    timespec remaining = SLEEP;
    while (::nanosleep(&remaining, &remaining) != 0) {
        // retry if interrupted by a signal
    }
    const auto t1 = func();
    remaining = SLEEP;
    while (::nanosleep(&remaining, &remaining) != 0) {
        // retry if interrupted by a signal
    }
    const auto t2 = func();
    const uint64_t diff = t2 - t1;
    EXPECT_GE(diff, threshold_ticks);
}

} // namespace

TEST(NowTest, nano_now) {
    verifyNow(nano_now);
    verifyUniformity(nano_now, 50 * NSECS_PER_MSEC);
}

TEST(NowTest, micro_now) {
    verifyNow(micro_now);
    verifyUniformity(micro_now, 50 * USECS_PER_MSEC);
}

TEST(NowTest, milli_now) {
    verifyNow(milli_now);
    verifyUniformity(milli_now, 50);
}

} // namespace brasa::chronus
