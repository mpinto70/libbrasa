#include "CircularHelper.h"
#include "brasa/buffer/CircularWriter.h"

#include <gtest/gtest.h>

namespace brasa {
namespace buffer {
namespace {
using data = SimpleData;
}

TEST(CircularWriter, read) {
    uint8_t buffer[CircularHelper<data, 30>::BUFFER_SIZE] = {};
    const CircularHelper<data, 30> reader(buffer, 12345);
    const CircularWriter<data, 30> writer(buffer, 12345);

    data t1, t2;

    t1.character = 'c';
    t1.value = 555444;

    writer.write(t1);

    EXPECT_TRUE(reader.read(t2));

    EXPECT_EQ(t1, t2);
}
}
}
