#include "brasa/chronus/NowStd.h"
#include "brasa/chronus/Chronometer.h"

#include <gtest/gtest.h>

namespace brasa {
namespace chronus {
namespace {
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
}

TEST(ChronometerTest, create) {
    const auto chron1 = make_chronometer(NanoNow, 1);
    EXPECT_EQ(chron1.id(), 1u);
    const auto chron2 = make_chronometer(NanoNow, 7865);
    EXPECT_EQ(chron2.id(), 7865u);
}

TEST(ChronometerTest, mark) {
    const auto chron = make_chronometer(NanoNow, 1234);
    const auto t1 = chron.mark(4321);
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

TEST(ChronometerTest, reset) {
    auto chron = make_chronometer(NowFunctor(100), 1234);
    const auto t1 = chron.mark(4321);
    chron.reset();
    const auto t2 = chron.mark(32);

    EXPECT_EQ(t1.chrono_id, 1234u);
    EXPECT_EQ(t1.mark_id, 4321u);
    EXPECT_EQ(t2.chrono_id, 1234u);
    EXPECT_EQ(t2.mark_id, 32u);

    EXPECT_EQ(t1.begin, 100u);
    EXPECT_EQ(t1.end, 101u);
    EXPECT_EQ(t2.begin, 102u);
    EXPECT_EQ(t2.end, 103u);
}

}
}
