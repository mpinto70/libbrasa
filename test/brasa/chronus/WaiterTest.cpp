#include "brasa/chronus/Sleeper.h"
#include "brasa/chronus/NowStd.h"
#include "brasa/chronus/Waiter.h"

#include <gtest/gtest.h>

#include <ctime>

namespace brasa {
namespace chronus {
namespace {
template <typename NOW_FUNC, typename SLEEPER>
void verifyWait(NOW_FUNC func, SLEEPER sleeper) {
    auto waiter = make_waiter<NOW_FUNC, SLEEPER>(std::move(func), 1 * NSECS_PER_MSEC, std::move(sleeper));
    for (unsigned i = 0; i < 100; ++i) {
        const auto t0 = NowStd();
        waiter.reset();
        EXPECT_FALSE(waiter.elapsed());
        waiter.wait();
        EXPECT_TRUE(waiter.elapsed());
        const auto t1 = NowStd();
        uint64_t diff = t1 - t0;
        EXPECT_GT(diff, 1 * NSECS_PER_MSEC) << "iteration " << i;
        EXPECT_LT(diff, 10 * NSECS_PER_MSEC) << "iteration " << i;
    }
}
}

TEST(WaitTest, wait) {
    verifyWait(NowStd, NanoSleeper());
}

}
}
