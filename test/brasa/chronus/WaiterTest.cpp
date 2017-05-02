#include "brasa/chronus/Sleeper.h"
#include "brasa/chronus/NowStd.h"
#include "brasa/chronus/Waiter.h"

#include <gtest/gtest.h>

#include <ctime>

namespace brasa {
namespace chronus {
namespace {
template <typename NOW_FUNC, typename SLEEPER>
void verifyWait(NOW_FUNC now_func, SLEEPER sleeper_func, const uint32_t wait_size) {
    NOW_FUNC now = now_func;
    auto waiter = make_waiter<NOW_FUNC, SLEEPER>(std::move(now_func), wait_size, std::move(sleeper_func));
    for (unsigned i = 0; i < 100; ++i) {
        const auto t0 = now();
        waiter.reset();
        EXPECT_FALSE(waiter.elapsed());
        waiter.wait();
        EXPECT_TRUE(waiter.elapsed());
        const auto t1 = now();
        uint64_t diff = t1 - t0;
        EXPECT_GT(diff, wait_size) << "iteration " << i;
        EXPECT_LT(diff, 10 * wait_size) << "iteration " << i;
    }
}
}

TEST(WaitTest, nano) {
    verifyWait(NanoNow, NanoSleeper(), 1 * NSECS_PER_MSEC);
}

TEST(WaitTest, micro) {
    verifyWait(MicroNow, ::usleep, 1 * USECS_PER_MSEC);
}

}
}
