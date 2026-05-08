#include <brasa/chronus/Constants.h>
#include <brasa/chronus/Now.h>
#include <brasa/chronus/SleepStd.h>

#include <gtest/gtest.h>

namespace brasa::chronus {
namespace {
template <typename SLEEP_FUNC>
void verifySleep(SLEEP_FUNC func, const uint32_t nsecs_per_unit, const uint32_t sleep_size) {
    for (unsigned i = 0; i < 50; ++i) {
        const auto t0 = nano_now();
        func(sleep_size);
        const auto t1 = nano_now();
        uint64_t diff = t1 - t0;
        EXPECT_GT(diff, sleep_size * nsecs_per_unit) << "iteration " << i;
    }
}
} // namespace

TEST(SleepTest, nano_sleep) {
    verifySleep(nano_sleep, 1, 1 * NSECS_PER_MSEC);
    verifySleep(nano_sleep, 1, 2 * NSECS_PER_MSEC);
}

TEST(SleepTest, micro_sleep) {
    verifySleep(micro_sleep, NSECS_PER_USEC, 100);
    verifySleep(micro_sleep, NSECS_PER_USEC, 200);
}

TEST(SleepTest, milli_sleep) {
    verifySleep(milli_sleep, NSECS_PER_MSEC, 1);
    verifySleep(milli_sleep, NSECS_PER_MSEC, 2);
}
} // namespace brasa::chronus
