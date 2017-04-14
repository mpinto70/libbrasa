
#include "brasa/chronometer/Chronometer.h"

#include <gtest/gtest.h>

namespace chronometer {

TEST(ChronometerTest, create) {
    const Chronometer chron(1);
    EXPECT_EQ(chron.id(), 1u);
}

}

