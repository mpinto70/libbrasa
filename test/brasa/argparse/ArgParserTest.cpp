#include <brasa/argparse/ArgParser.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <sstream>

namespace brasa::argparse {
namespace {
template <size_t BUFFER_SIZE, size_t ARGV_SIZE>
void build(
      const std::vector<std::string>& options,
      char (&buffer)[BUFFER_SIZE],
      int& argc,
      char* (&argv)[ARGV_SIZE]) {
    ASSERT_LT(options.size(), ARGV_SIZE);
    std::fill(buffer, buffer + BUFFER_SIZE, '\0');
    std::fill(argv, argv + ARGV_SIZE, nullptr);
    argc = 0;
    size_t curr_idx = 0;
    for (size_t i = 0; i < options.size(); ++i) {
        const auto& option = options[i];
        ASSERT_LT(curr_idx + option.size() + 1, BUFFER_SIZE) << option;
        option.copy(&buffer[curr_idx], std::string::npos);
        argv[i] = &buffer[curr_idx];
        curr_idx += option.size() + 1;
        ++argc;
    }
    optind = 0;
}
} // namespace

TEST(ArgParserTest, test_parse) {
    auto parser = make_parser(
          "program description",
          std::make_tuple(
                SingleValue<std::string>("PARAMETER1", "a parameter"),
                SingleValue<std::string>("PARAMETER2", "another parameter"),
                SingleValue<std::string>("PARAMETER3", "yet another parameter")),
          std::make_tuple(
                BooleanParser('l', "list-files", "list the files"),
                ValueParser<SingleValue<std::string>>(
                      'i',
                      "ignore-file",
                      "file-to-ignore",
                      "mark the file to be ignored")));

    EXPECT_EQ(parser.VALUE_SIZE, 3u);
    EXPECT_EQ(parser.PARSER_SIZE, 2u);

    const std::vector<std::string> options = {
        "command", "parameter1", "-l", "-i", "file1.cpp", "parameter2", "bleh",
    };
    int argc = 0;
    char* argv[50];
    char buffer[4096];
    build(options, buffer, argc, argv);
    std::ostringstream out;
    EXPECT_EQ(parser.parse(argc, argv, out), ParseResult::Ok);
    EXPECT_EQ(out.str(), "");

    EXPECT_EQ(std::get<0>(parser.parsers()).value(), true);
    EXPECT_EQ(std::get<1>(parser.parsers()).is_present(), true);
    EXPECT_EQ(std::get<1>(parser.parsers()).digester().value(), "file1.cpp");
    EXPECT_EQ(std::get<0>(parser.values()).value(), "parameter1");
    EXPECT_EQ(std::get<1>(parser.values()).value(), "parameter2");
    EXPECT_EQ(std::get<2>(parser.values()).value(), "bleh");
}

TEST(ArgParserTest, test_usage_multiple_parameters_and_multiple_options) {
    const std::string description = "Some program description";
    const auto parser = make_parser(
          description,
          std::make_tuple(
                SingleValue<std::string>("PARAMETER1", "a parameter to process"),
                SingleValue<std::string>("PARAMETER2", "a second parameter to process")),
          std::make_tuple(
                BooleanParser('l', "list-files", "list files"),
                ValueParser<SingleValue<std::string>>(
                      'i',
                      "ignore-file",
                      "file-to-ignore",
                      "mark a file to be ignored")));

    const std::string expected_usage = "Some program description\n"
                                       "\n"
                                       "executable PARAMETER1 PARAMETER2 <options>\n"
                                       "\n"
                                       "Positional parameters:\n"
                                       "    PARAMETER1              a parameter to process\n"
                                       "    PARAMETER2              a second parameter to process\n"
                                       "Options:\n"
                                       "    -l, --list-files        list files\n"
                                       "    -i, --ignore-file  <file-to-ignore>\n"
                                       "                            mark a file to be ignored\n";
    EXPECT_EQ(parser.usage("executable"), expected_usage);
}

TEST(ArgParserTest, test_usage_multiple_no_parameters_and_multiple_options) {
    const std::string description = "Some program description";
    const auto parser = make_parser(
          description,
          std::make_tuple(),
          std::make_tuple(
                BooleanParser('l', "list-files", "list files"),
                ValueParser<SingleValue<std::string>>(
                      'i',
                      "ignore-file",
                      "file-to-ignore",
                      "mark a file to be ignored")));

    const std::string expected_usage = "Some program description\n"
                                       "\n"
                                       "executable <options>\n"
                                       "\n"
                                       "Options:\n"
                                       "    -l, --list-files        list files\n"
                                       "    -i, --ignore-file  <file-to-ignore>\n"
                                       "                            mark a file to be ignored\n";
    EXPECT_EQ(parser.usage("executable"), expected_usage);
}

TEST(ArgParserTest, test_usage_multiple_parameters_and_no_options) {
    const std::string description = "Some program description";
    const auto parser = make_parser(
          description,
          std::make_tuple(
                SingleValue<std::string>("PARAMETER1", "a parameter to process"),
                SingleValue<std::string>("PARAMETER2", "a second parameter to process")),
          std::make_tuple());

    const std::string expected_usage =
          "Some program description\n"
          "\n"
          "executable PARAMETER1 PARAMETER2\n"
          "\n"
          "Positional parameters:\n"
          "    PARAMETER1              a parameter to process\n"
          "    PARAMETER2              a second parameter to process\n";
    EXPECT_EQ(parser.usage("executable"), expected_usage);
}

TEST(ArgParserTest, test_usage_footer) {
    const std::string description = "Some program description";
    const std::string footer = "Some footer";
    const auto parser = make_parser(
          description,
          std::make_tuple(
                SingleValue<std::string>("PARAMETER1", "a parameter to process"),
                SingleValue<std::string>("PARAMETER2", "a second parameter to process")),
          std::make_tuple(
                BooleanParser('l', "list-files", "list files"),
                ValueParser<SingleValue<std::string>>(
                      'i',
                      "ignore-file",
                      "file-to-ignore",
                      "mark a file to be ignored")),
          footer);

    const std::string expected_usage = "Some program description\n"
                                       "\n"
                                       "executable PARAMETER1 PARAMETER2 <options>\n"
                                       "\n"
                                       "Positional parameters:\n"
                                       "    PARAMETER1              a parameter to process\n"
                                       "    PARAMETER2              a second parameter to process\n"
                                       "Options:\n"
                                       "    -l, --list-files        list files\n"
                                       "    -i, --ignore-file  <file-to-ignore>\n"
                                       "                            mark a file to be ignored\n"
                                       "\n"
                                       "Some footer\n";
    EXPECT_EQ(parser.usage("executable"), expected_usage);
}

namespace {
auto create_parser() {
    return make_parser(
          "program description",
          std::make_tuple(
                SingleValue<std::string>("PARAMETER1", "a parameter"),
                SingleValue<std::string>("PARAMETER2", "another parameter"),
                SingleValue<int>("PARAMETER3", "yet another parameter")),
          std::make_tuple(
                BooleanParser('l', "list-files", "list the files"),
                ValueParser<SingleValue<std::string>>(
                      'i',
                      "ignore-file",
                      "file-to-ignore",
                      "mark the file to be ignored"),
                ValueParser<SingleValue<int>>(
                      'n',
                      "number",
                      "number-of-times",
                      "set the number of times")));
}

void check_error_in_parser(const std::vector<std::string>& options, const std::string& error_msg) {
    auto parser = create_parser();

    int argc = 0;
    char* argv[50];
    char buffer[4096];
    build(options, buffer, argc, argv);
    std::ostringstream out;
    EXPECT_EQ(parser.parse(argc, argv, out), ParseResult::Error);
    EXPECT_EQ(
          out.str(),
          error_msg
                + "\n"
                  "\n"
                + parser.usage(options[0]));
}
} // namespace

TEST(ArgParserTest, test_parse_error_insufficient_values) {
    const std::vector<std::string> options = {
        "command",
        "parameter1",
        "parameter2",
    };

    check_error_in_parser(
          options,
          "ERROR processing command line arguments: missing arguments for PARAMETER3");
}

TEST(ArgParserTest, test_parse_error_too_many_values) {
    const std::vector<std::string> options = {
        "command", "parameter1", "parameter2", "789", "excess", "value",
    };

    check_error_in_parser(
          options,
          "ERROR processing command line arguments: too many arguments 'excess' 'value'");
}

TEST(ArgParserTest, test_parse_error_incorrect_vlue_type) {
    const std::vector<std::string> options = {
        "command",
        "parameter1",
        "parameter2",
        "text",
    };

    check_error_in_parser(
          options,
          "ERROR processing command line arguments: could not convert value 'text' of PARAMETER3");
}

} // namespace brasa::argparse
