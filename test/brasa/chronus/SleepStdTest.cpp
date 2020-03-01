#include <brasa/chronus/Constants.h>
#include <brasa/chronus/Now.h>
#include <brasa/chronus/SleepStd.h>

#include <gtest/gtest.h>
#include <unistd.h>

#include <ctime>

namespace brasa {
namespace chronus {
namespace {
template <typename SLEEP_FUNC>
void veriffySleep(SLEEP_FUNC func, const uint32_t MULT_NSEC, const uint32_t sleep_size) {
    for (unsigned i = 0; i < 100; ++i) {
        const auto t0 = nano_now();
        func(sleep_size);
        const auto t1 = nano_now();
        uint64_t diff = t1 - t0;
        EXPECT_GT(diff, sleep_size * MULT_NSEC) << "iteration " << i;
    }
}
}

TEST(SleepTest, usleep) {
    veriffySleep(::usleep, NSECS_PER_USEC, 100);
    veriffySleep(::usleep, NSECS_PER_USEC, 200);
}

TEST(SleepTest, nano_sleep) {
    veriffySleep(nano_sleep, 1, 1 * NSECS_PER_MSEC);
    veriffySleep(nano_sleep, 1, 2 * NSECS_PER_MSEC);
}

TEST(SleepTest, micro_sleep) {
    veriffySleep(micro_sleep, NSECS_PER_USEC, 100);
    veriffySleep(micro_sleep, NSECS_PER_USEC, 200);
}

TEST(SleepTest, milli_sleep) {
    veriffySleep(milli_sleep, NSECS_PER_MSEC, 1);
    veriffySleep(milli_sleep, NSECS_PER_MSEC, 2);
}
}
}
