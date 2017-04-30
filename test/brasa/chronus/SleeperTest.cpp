#include "brasa/chronus/Sleeper.h"
#include "brasa/chronus/Constants.h"
#include "brasa/chronus/NowStd.h"

#include <gtest/gtest.h>
#include <unistd.h>

#include <ctime>

namespace brasa {
namespace chronus {
namespace {
template <typename SLEEP_FUNC>
void veriffySleep(SLEEP_FUNC func, const uint32_t MULT_NSEC, const uint32_t sleep_size) {
    for (unsigned i = 0; i < 100; ++i) {
        const auto t0 = NowStd();
        func(sleep_size);
        const auto t1 = NowStd();
        uint64_t diff = t1 - t0;
        EXPECT_GT(diff, sleep_size * MULT_NSEC) << "iteration " << i;
    }
}
}

TEST(SleepTest, usleep) {
    veriffySleep(::usleep, NSECS_PER_USEC, 100);
    veriffySleep(::usleep, NSECS_PER_USEC, 200);
}

TEST(SleepTest, nanosleep) {
    veriffySleep(NanoSleeper(), 1, 1 * NSECS_PER_MSEC);
    veriffySleep(NanoSleeper(), 1, 2 * NSECS_PER_MSEC);
}

}
}
