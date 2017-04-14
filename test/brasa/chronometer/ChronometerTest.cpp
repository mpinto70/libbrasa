
#include "brasa/chronometer/Chronometer.h"

#include <gtest/gtest.h>

#include <chrono>
#include <ctime>

namespace chronometer {
namespace {
uint64_t Now() {
    return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

class NowFunctor {
public:
    NowFunctor(uint64_t initial)
        : initial_(initial) {
    }
    uint64_t operator()() const {
        return initial_++;
    }
private:
    mutable uint64_t initial_;
};

constexpr timespec SLEEP = {0, 50000}; ///< 50 microseconds
}

TEST(ChronometerTest, create) {
    const auto chron1 = make_chronometer(Now, 1);
    EXPECT_EQ(chron1.id(), 1u);
    const auto chron2 = make_chronometer(Now, 7865);
    EXPECT_EQ(chron2.id(), 7865u);
}

TEST(ChronometerTest, mark) {
    const auto chron = make_chronometer(Now, 1234);
    EXPECT_EQ(::nanosleep(&SLEEP, nullptr), 0);
    const auto t1 = chron.mark(4321);
    EXPECT_EQ(::nanosleep(&SLEEP, nullptr), 0);
    const auto t2 = chron.mark(32);

    EXPECT_EQ(t1.chrono_id, 1234u);
    EXPECT_EQ(t1.mark_id, 4321u);
    EXPECT_EQ(t2.chrono_id, 1234u);
    EXPECT_EQ(t2.mark_id, 32u);

    EXPECT_EQ(t1.begin, t2.begin);
    EXPECT_LT(t1.end, t2.end);
}

TEST(ChronometerTest, markWithFunctor) {
    const auto chron = make_chronometer(NowFunctor(48), 1234);
    const auto t1 = chron.mark(4321);
    const auto t2 = chron.mark(32);

    EXPECT_EQ(t1.chrono_id, 1234u);
    EXPECT_EQ(t1.mark_id, 4321u);
    EXPECT_EQ(t2.chrono_id, 1234u);
    EXPECT_EQ(t2.mark_id, 32u);

    EXPECT_EQ(t1.begin, t2.begin);
    EXPECT_EQ(t1.begin, 48u);
    EXPECT_EQ(t1.end, 49u);
    EXPECT_EQ(t2.end, 50u);
}

}

