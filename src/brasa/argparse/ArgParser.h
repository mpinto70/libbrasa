#pragma once

#include <brasa/argparse/Parsers.h>

#include <getopt.h>

#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

namespace brasa::argparse {

namespace detail {
template <typename FunctorT, size_t I = 0, typename TupleT, typename... Ts>
typename std::enable_if<I == std::tuple_size_v<TupleT>, void>::type for_each_tuple(
      TupleT&,
      Ts&&...) {}

template <typename FunctorT, size_t I = 0, typename TupleT, typename... Ts>
      typename std::enable_if
      < I<std::tuple_size_v<TupleT>, void>::type for_each_tuple(TupleT& a_tuple, Ts&&... ts) {
    FunctorT f;
    f.template operator()<I>(a_tuple, ts...);
    for_each_tuple<FunctorT, I + 1, TupleT>(a_tuple, std::forward<Ts>(ts)...);
}

template <
      typename FunctorT,
      typename ReturnT,
      typename BinaryOperator = std::plus<ReturnT>,
      size_t I = 0,
      typename TupleT,
      typename... Ts>
typename std::enable_if<I == std::tuple_size_v<TupleT>, ReturnT>::type
      accumulate_tuple(TupleT&, const ReturnT& return_v = ReturnT{}, Ts&&...) {
    return return_v;
}

template <
      typename FunctorT,
      typename ReturnT,
      typename BinaryOperator = std::plus<ReturnT>,
      size_t I = 0,
      typename TupleT,
      typename... Ts>
      typename std::enable_if < I<std::tuple_size_v<TupleT>, ReturnT>::type
      accumulate_tuple(TupleT& a_tuple, ReturnT return_v = ReturnT{}, Ts&&... ts) {
    FunctorT f;
    return_v = BinaryOperator{}(return_v, f.template operator()<I>(a_tuple, ts...));
    return accumulate_tuple<FunctorT, ReturnT, BinaryOperator, I + 1, TupleT>(
          a_tuple,
          return_v,
          std::forward<Ts>(ts)...);
}
} // namespace detail

enum class ParseResult {
    Ok,
    Error,
    Help,
};

/** A class to parse command line arguments.
 * @tparam ValueTupleT a tuple of objects to process mandatory arguments
 * @tparam ParserTupleT a tuple of objects to process optional arguments
 *
 * Each element of ValueTupleT should have functions:
 * - `void digest(const std::string& argument)` to digest the argument and store the relevant value
 * - `<type> value() const` to return the value parsed (and possibly converted)
 * - `bool can_digest() const` that return if it is still possible to digest an argument. This
 *      function can also be static instead of const
 * - `std::string name() const` to return the name of the parameter to be used in documenting usage
 *      and reporting errors
 * - `std::string description() const` to return the description of the parameter to be used in
 *      documenting usage
 *
 * Each element of ParseTupleT should either be a BooleanParser or have:
 * - `void digest(const std::string& argument)` to digest the argument and store the relevant value
 * - `<type> value() const` to return the value parsed (and possibly converted)
 * - `bool is_present() const` that return if the argument was present in command line
 * - `std::string name() const` to return the name of the parameter to be used in documenting usage
 * - `std::string description() const` to return the description of the parameter to be used in
 *      documenting usage
 * - `char short_option() const` to return the short option (it has to be unique and cannot be `h`)
 * - `const std::string& long_option() const` to return the long option (it has to be unique and
 * cannot be `help` and also has to be persistent so it can be pointed to by the command line
 * parser) Also, ParseTupleT elements that are not BooleanParser should define `constexpr static
 * bool IS_BOOLEAN = false;`
 */
template <typename ValueTupleT, typename ParserTupleT>
class ArgParser {
public:
    static constexpr size_t VALUE_SIZE = std::tuple_size_v<ValueTupleT>;
    static constexpr size_t PARSER_SIZE = std::tuple_size_v<ParserTupleT>;
    ArgParser(
          std::string description,
          ValueTupleT value_tuple,
          ParserTupleT parser_tuple,
          std::string footer)
          : description_(std::move(description)),
            values_(std::move(value_tuple)),
            parsers_(std::move(parser_tuple)),
            footer_(std::move(footer)) {
        std::vector<struct option> long_options;
        std::string short_options;
        long_options.reserve(PARSER_SIZE + 2);
        short_options.reserve(PARSER_SIZE * 2 + 1);

        detail::for_each_tuple<BuildOptions>(parsers_, long_options, short_options);

        const auto is_help = [](const struct option& op) {
            return std::strcmp(op.name, "help") == 0;
        };
        if (short_options.find('h') != std::string::npos
            || std::find_if(long_options.begin(), long_options.end(), is_help)
                     != long_options.end()) {
            throw InvalidArgument("Option -h/--help is reserved for ArgParser");
        }

        short_options += 'h';
        long_options.push_back({ HELP_, no_argument, nullptr, 'h' });
        long_options.push_back({ nullptr, 0, nullptr, '\0' });

        short_options_.swap(short_options);
        long_options_.swap(long_options);
    }

    const ValueTupleT& values() const { return values_; }
    const ParserTupleT& parsers() const { return parsers_; }

    std::string usage(const std::string& executable) const {
        const auto parameters_usage = detail::accumulate_tuple<BuildUsage, std::string>(values_);
        const auto parsers_usage = detail::accumulate_tuple<BuildUsage, std::string>(parsers_);
        const auto command_line_parameters =
              detail::accumulate_tuple<BuildCommandLine, std::string>(values_);
        std::string res = description_;
        res += "\n\n";
        res += executable;
        res += command_line_parameters;
        res += (PARSER_SIZE > 0 ? " <options>" : "");
        res += "\n\n";
        res += parameters_usage;
        res += parsers_usage;
        res += footer_.empty() ? "" : "\n" + footer_ + "\n";

        return res;
    }

    ParseResult parse(int argc, char* const argv[], std::ostream& err) noexcept {
        try {
            const auto parse_result = do_parse(argc, argv, err);
            switch (parse_result) {
                case ParseResult::Ok:
                    break;
                case ParseResult::Error:
                case ParseResult::Help:
                    err << usage(argv[0]);
                    break;
            }
            return parse_result;
        } catch (const InvalidArgument& error) {
            err << ERROR_MSG_ << error.what() << "\n\n" << usage(argv[0]);
            return ParseResult::Error;
        }
    }

private:
    std::string description_;
    ValueTupleT values_;
    ParserTupleT parsers_;
    std::string footer_;
    std::vector<struct option> long_options_;
    std::string short_options_;

    static constexpr const char HELP_[] = "help";
    static constexpr const char ERROR_MSG_[] = "ERROR processing command line arguments: ";

private: // functions
    ParseResult do_parse(int argc, char* const argv[], std::ostream& err) {
        int c;
        while ((c = getopt_long(argc, argv, short_options_.c_str(), long_options_.data(), nullptr))
               != -1) {
            switch (c) {
                case 'h':
                    return ParseResult::Help;
                case '?':
                    return ParseResult::Error;
                default:
                    if (optarg == nullptr) {
                        detail::for_each_tuple<ParseOption>(parsers_, c, "");
                    } else {
                        detail::for_each_tuple<ParseOption>(parsers_, c, optarg);
                    }
            }
        }

        int index = optind;
        const size_t first_value_index = index;
        if (not detail::accumulate_tuple<ParseValues, bool, std::logical_and<bool>>(
                  values_,
                  true,
                  index,
                  argc,
                  argv)) {
            const size_t num_values_processed = index - first_value_index;
            const auto missing_arguments =
                  detail::accumulate_tuple<BuildMissingParameters, std::string>(
                        values_,
                        "",
                        num_values_processed);
            err << ERROR_MSG_ << "missing arguments for" << missing_arguments << "\n\n";
            return ParseResult::Error;
        }

        if (index == argc) { // everything was consumed without error
            return ParseResult::Ok;
        } else { // there are still arguments in the command line
            err << ERROR_MSG_ << "too many arguments";
            for (; index < argc; ++index) {
                err << " '" << argv[index] << "'";
            }
            err << "\n\n";
            return ParseResult::Error;
        }
    }

    struct ParseOption {
        template <size_t I, typename TupleT>
        void operator()(TupleT& parsers, char short_option, const std::string& parameter) {
            auto& parser = std::get<I>(parsers);
            if (short_option == parser.short_option()) {
                if constexpr (parser.IS_BOOLEAN) {
                    parser.digest();
                } else {
                    parser.digest(parameter);
                }
            }
        }
    };

    struct ParseValues {
        template <size_t I>
        bool operator()(ValueTupleT& values, int& index, const int argc, char* const argv[]) {
            if (index >= argc) {
                return false;
            }
            auto& value = std::get<I>(values);
            while (value.can_digest() && index < argc) {
                value.digest(argv[index]);
                ++index;
            }
            return true;
        }
    };

    struct BuildUsage {
        static std::string build_usage_line(std::string preamble, std::string description) {
            constexpr size_t SECOND_COLUMN = 24;
            const std::string indent = "    ";
            if (preamble.length() < SECOND_COLUMN) {
                preamble += std::string(SECOND_COLUMN - preamble.length(), ' ');
            } else {
                preamble += "\n";
                description = std::string(SECOND_COLUMN + indent.length(), ' ') + description;
            }
            return indent + preamble + description + "\n";
        }

        template <size_t I>
        std::string operator()(const ParserTupleT& parsers) {
            std::string usage = (I == 0) ? "Options:\n" : "";
            auto& parser = std::get<I>(parsers);
            const std::string& long_option = parser.long_option();
            const char short_option = parser.short_option();

            std::string preamble = "-" + std::string(1, short_option) + ", --" + long_option;
            if constexpr (not parser.IS_BOOLEAN) {
                preamble += "  <" + parser.name() + ">";
            }

            usage += build_usage_line(preamble, parser.description());

            return usage;
        }

        template <size_t I>
        std::string operator()(const ValueTupleT& values) {
            std::string usage = (I == 0) ? "Positional parameters:\n" : "";

            auto& value = std::get<I>(values);

            usage += build_usage_line(value.name(), value.description());

            return usage;
        }
    };

    struct BuildCommandLine {
        template <size_t I>
        std::string operator()(const ValueTupleT& values) {
            auto& value = std::get<I>(values);
            return " " + value.name();
        }
    };

    struct BuildOptions {
        template <size_t I>
        void operator()(
              const ParserTupleT& parsers,
              std::vector<struct option>& long_options,
              std::string& short_options) {
            auto& parser = std::get<I>(parsers);
            const std::string& long_option = parser.long_option();
            const char short_option = parser.short_option();
            const auto require_argument = parser.IS_BOOLEAN ? no_argument : required_argument;
            const struct option opt = { long_option.c_str(),
                                        require_argument,
                                        nullptr,
                                        short_option };
            const auto is_present = [&long_option](const struct option& op) {
                return long_option == op.name;
            };

            if (short_options.find(short_option) != std::string::npos
                || std::find_if(long_options.begin(), long_options.end(), is_present)
                         != long_options.end()) {
                throw InvalidArgument(
                      "Duplicated option: -" + std::string(1, short_option) + "/--" + long_option);
            }

            long_options.push_back(opt);
            short_options += short_option;
            if (not parser.IS_BOOLEAN) {
                short_options += ':';
            }
        }
    };

    struct BuildMissingParameters {
        template <size_t I>
        std::string operator()(const ValueTupleT& values, size_t num_values_processed) {
            if (I >= num_values_processed) {
                auto& value = std::get<I>(values);
                return " " + value.name();
            } else {
                return "";
            }
        }
    };
};

template <typename ValueTupleT, typename ParserTupleT>
ArgParser<ValueTupleT, ParserTupleT> make_parser(
      std::string description,
      ValueTupleT value_tuple,
      ParserTupleT parser_tuple,
      std::string footer = "") {
    return ArgParser<ValueTupleT, ParserTupleT>(
          std::move(description),
          std::move(value_tuple),
          std::move(parser_tuple),
          std::move(footer));
}
} // namespace brasa::argparse
