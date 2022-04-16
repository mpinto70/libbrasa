#include <brasa/argparse/Parsers.h>

namespace brasa::argparse {

BaseParser::BaseParser(char short_option, std::string long_option, std::string description)
      : short_option_(short_option),
        long_option_(std::move(long_option)),
        description_(std::move(description)) {}

BooleanParser::BooleanParser(char short_option, std::string long_option, std::string description)
      : BaseParser(short_option, std::move(long_option), std::move(description)),
        active_(false) {}

}
