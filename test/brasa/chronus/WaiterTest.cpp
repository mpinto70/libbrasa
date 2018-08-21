#include "brasa/chronus/Waiter.h"
#include "brasa/chronus/NowStd.h"
#include <brasa/chronus/SleepStd.h>

#include <gtest/gtest.h>

#include <ctime>

namespace brasa {
namespace chronus {
namespace {
template <typename NOW_FUNC, typename SLEEPER_FUNC>
void verifyWait(NOW_FUNC now_func,
      SLEEPER_FUNC sleeper_func,
      const uint32_t wait_size) {
    NOW_FUNC now = now_func;
    auto waiter = make_waiter<NOW_FUNC, SLEEPER_FUNC>(std::move(now_func), wait_size, std::move(sleeper_func));
    for (unsigned i = 0; i < 100; ++i) {
        const auto t0 = now();
        waiter.reset();
        EXPECT_FALSE(waiter.elapsed());
        waiter.wait();
        EXPECT_TRUE(waiter.elapsed());
        const auto t1 = now();
        uint64_t diff = t1 - t0;
        EXPECT_GE(diff, wait_size) << "iteration " << i;
    }
}
}

TEST(WaitTest, Nano) {
    verifyWait(nano_now, nano_sleep, 1 * NSECS_PER_MSEC);
}

TEST(WaitTest, Micro) {
    verifyWait(micro_now, micro_sleep, 500);
}

TEST(WaitTest, Milli) {
    verifyWait(milli_now, milli_sleep, 1);
}
}
}
