#include <brasa/argparse/Parsers.h>

#include <gtest/gtest.h>

namespace brasa::argparse {
TEST(BooleanParserTest, test_creation) {
    BooleanParser bool_arg('l', "list-files", "some description");
    EXPECT_EQ(bool_arg.value(), false);
    bool_arg.digest();
    EXPECT_EQ(bool_arg.value(), true);
    bool_arg.digest();
    EXPECT_EQ(bool_arg.value(), true);
}

TEST(ValueParserTest, test_name) {
    const ValueParser<SingleValue<std::string>> arg1(
          'i',
          "ignore-file",
          "file-to-ignore",
          "some description");
    const ValueParser<SingleValue<std::string>> arg2(
          'o',
          "option",
          "option-to-apply",
          "some description");
    EXPECT_EQ(arg1.name(), "file-to-ignore");
    EXPECT_EQ(arg2.name(), "option-to-apply");
}

TEST(ValueParserTest, test_creation_single_string) {
    ValueParser<SingleValue<std::string>> arg(
          'i',
          "ignore-file",
          "file-to-ignore",
          "some description");
    EXPECT_EQ(arg.digester().value(), "");
    arg.digest("something");
    EXPECT_EQ(arg.digester().value(), "something");
}

TEST(ValueParserTest, test_creation_multiple_string) {
    ValueParser<MultiValue<std::string>> arg(
          'i',
          "ignore-file",
          "file-to-ignore",
          "some description");

    arg.digest("something");
    EXPECT_EQ(arg.digester().value(0), "something");

    arg.digest("other thing");
    EXPECT_EQ(arg.digester().value(0), "something");
    EXPECT_EQ(arg.digester().value(1), "other thing");
}

TEST(ValueParserTest, test_creation_single_int) {
    ValueParser<SingleValue<int>> arg('i', "ignore-file", "file-to-ignore", "some description");
    EXPECT_EQ(arg.digester().value(), 0);
    arg.digest("7");
    EXPECT_EQ(arg.digester().value(), 7);
}

}
