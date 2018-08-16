#include "brasa/chronus/SleepStd.h"
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
        const auto t0 = NanoNow();
        func(sleep_size);
        const auto t1 = NanoNow();
        uint64_t diff = t1 - t0;
        EXPECT_GT(diff, sleep_size * MULT_NSEC) << "iteration " << i;
    }
}
}

TEST(SleepTest, usleep) {
    veriffySleep(::usleep, NSECS_PER_USEC, 100);
    veriffySleep(::usleep, NSECS_PER_USEC, 200);
}

TEST(SleepTest, NanoSleep) {
    veriffySleep(NanoSleep, 1, 1 * NSECS_PER_MSEC);
    veriffySleep(NanoSleep, 1, 2 * NSECS_PER_MSEC);
}

TEST(SleepTest, MicroSleep) {
    veriffySleep(MicroSleep, NSECS_PER_USEC, 100);
    veriffySleep(MicroSleep, NSECS_PER_USEC, 200);
}

TEST(SleepTest, MilliSleep) {
    veriffySleep(MilliSleep, NSECS_PER_MSEC, 1);
    veriffySleep(MilliSleep, NSECS_PER_MSEC, 2);
}
}
}
