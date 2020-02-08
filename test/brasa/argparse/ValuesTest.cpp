#include "brasa/argparse/Values.h"

#include <gtest/gtest.h>

namespace brasa::argparse {
TEST(SingleValueTest, test_string) {
    SingleValue<std::string> str("some name", "some description");
    EXPECT_EQ(str.value(), "");
    EXPECT_TRUE(str.can_digest());
    EXPECT_EQ(str.name(), "some name");
    EXPECT_EQ(str.description(), "some description");

    str.digest("something");
    EXPECT_FALSE(str.can_digest());
    EXPECT_EQ(str.value(), "something");

    EXPECT_THROW(str.digest("something else"), InvalidArgument);
    EXPECT_FALSE(str.can_digest());
    EXPECT_EQ(str.value(), "something");
}

TEST(MultiValueTest, test_string) {
    MultiValue<std::string> strs("name", "some description");

    EXPECT_TRUE(strs.can_digest());
    EXPECT_EQ(strs.name(), "name");
    EXPECT_EQ(strs.description(), "some description");

    strs.digest("something");
    EXPECT_TRUE(strs.can_digest());
    EXPECT_EQ(strs.value(0), "something");

    strs.digest("something else");
    EXPECT_TRUE(strs.can_digest());
    EXPECT_EQ(strs.value(0), "something");
    EXPECT_EQ(strs.value(1), "something else");
}

TEST(SingleValueTest, test_int) {
    SingleValue<int> the_int("int name", "int description");
    EXPECT_EQ(the_int.value(), 0);
    EXPECT_EQ(the_int.name(), "int name");
    EXPECT_EQ(the_int.description(), "int description");

    the_int.digest("117");
    EXPECT_FALSE(the_int.can_digest());
    EXPECT_EQ(the_int.value(), 117);

    EXPECT_THROW(the_int.digest("12"), InvalidArgument);
    EXPECT_FALSE(the_int.can_digest());
    EXPECT_EQ(the_int.value(), 117);
}

TEST(MultiValueTest, test_int) {
    MultiValue<int> ints("ints name", "ints description");

    EXPECT_EQ(ints.name(), "ints name");
    EXPECT_EQ(ints.description(), "ints description");

    ints.digest("10");
    EXPECT_EQ(ints.value(0), 10);

    ints.digest("38");
    EXPECT_EQ(ints.value(0), 10);
    EXPECT_EQ(ints.value(1), 38);
}

TEST(LimitedMultiValueTest, test_int) {
    LimitedMultiValue<3, int> ints("ints name", "ints description");

    EXPECT_EQ(ints.name(), "ints name");
    EXPECT_EQ(ints.description(), "ints description");

    ints.digest("10");
    EXPECT_EQ(ints.value(0), 10);

    ints.digest("38");
    EXPECT_EQ(ints.value(0), 10);
    EXPECT_EQ(ints.value(1), 38);

    ints.digest("-74");
    EXPECT_EQ(ints.value(0), 10);
    EXPECT_EQ(ints.value(1), 38);
    EXPECT_EQ(ints.value(2), -74);

    EXPECT_THROW(ints.digest("12"), InvalidArgument);
}

TEST(SingleValueTest, test_int_invalid_conversion) {
    SingleValue<int> str("for invalid input", "for invalid input");
    EXPECT_EQ(str.value(), 0);
    EXPECT_THROW(str.digest("a"), InvalidArgument);
    EXPECT_EQ(str.value(), 0);
}

}
