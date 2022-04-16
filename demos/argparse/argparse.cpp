#include "brasa/argparse/ArgParser.h"
#include "brasa/argparse/Parsers.h"
#include "brasa/argparse/Values.h"

#include <iostream>
#include <string>

namespace {
void execute_command(
      const std::string& param1,
      int param2,
      bool is_list,
      const std::vector<std::string>& ignores) {
    using namespace std::literals;
    std::cout << "Executing command with " << param1 << " " << param2
              << (is_list ? " " : " without ") << "listing files ";
    if (ignores.empty()) {
        std::cout << "without ignores";
    } else {
        std::cout << "ignoring ";
        for (const auto& ignore : ignores) {
            std::cout << ignore << " ";
        }
    }
    std::cout << "\n";
}
}

int main(int argc, char* argv[]) {
    using brasa::argparse::BooleanParser;
    using brasa::argparse::make_parser;
    using brasa::argparse::MultiValue;
    using brasa::argparse::ParseResult;
    using brasa::argparse::SingleValue;
    using brasa::argparse::ValueParser;
    const std::string program_description = "Demo for argparse facility of libbrasa";
    auto parser = make_parser(
          program_description,
          std::make_tuple(
                SingleValue<std::string>("PARAMETER1", "a parameter to process"),
                SingleValue<int>("PARAMETER2", "a second parameter to process")),
          std::make_tuple(
                BooleanParser('l', "list-files", "list files"),
                ValueParser<MultiValue<std::string>>(
                      'i',
                      "ignore-file",
                      "file-to-ignore",
                      "mark a file to be ignored")));

    switch (parser.parse(argc, argv, std::cerr)) {
        case ParseResult::Ok: {
            const std::string param1 = std::get<0>(parser.values()).value();
            const int param2 = std::get<1>(parser.values()).value();
            const bool is_list = std::get<0>(parser.parsers()).value();
            const auto& ignore_parser = std::get<1>(parser.parsers());
            const auto ignores = ignore_parser.digester().values();
            execute_command(param1, param2, is_list, ignores);
            break;
        }
        case ParseResult::Error:
            std::cerr << "Parser returned with error\n";
            break;

        case ParseResult::Help:
            std::cerr << "Asked for help\n";
            break;
    }
}
