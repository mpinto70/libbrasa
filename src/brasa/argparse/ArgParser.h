/**
 * A sophisticated template-based command line argument parser for C++.
 *
 * This file provides a flexible and type-safe argument parsing system that supports
 * both positional and optional command line arguments. The parser uses templates
 * to provide compile-time type safety and allows for custom argument processors.
 */

#pragma once

#include <brasa/argparse/Parsers.h>

#include <getopt.h>

#include <cstring>
#include <format>
#include <functional>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

namespace brasa::argparse {

/**
 * Internal implementation details for the `ArgParser`.
 *
 * This namespace contains utility functions and classes used internally
 * by the `ArgParser`. These should not be used directly by client code.
 */
namespace detail {
/** Perform operation `FunctorT` to each element to `a_tuple` recursively.
 * @tparam FunctorT the functor to be applied to each element of the tuple
 * @tparam I the index of the element to be processed
 * @tparam TupleT the type tuple to be processed
 * @tparam Ts the types of the arguments to be passed to `FunctorT`
 * @param a_tuple the tuple to be processed
 * @param ts the arguments to be passed to `FunctorT`
 */
template <typename FunctorT, size_t I = 0, typename TupleT, typename... Ts>
void for_each_tuple(TupleT& a_tuple, Ts&&... ts) {
    if constexpr (I < std::tuple_size_v<TupleT>) {
        FunctorT f;
        f.template operator()<I>(a_tuple, ts...);
        for_each_tuple<FunctorT, I + 1, TupleT>(a_tuple, std::forward<Ts>(ts)...);
    }
}

/**
 * Cumulatively apply binary function `FunctorT` to each element of `a_tuple`.
 * @tparam FunctorT the function to be applied to each element of the tuple
 * @tparam ReturnT the type of the return value of `FunctorT`
 * @tparam BinaryOperator the binary operator to accumulate the returned values of `FunctorT`
 * @tparam I the index of the element to be processed
 * @tparam TupleT the type tuple to be processed
 * @tparam Ts the types of the arguments to be passed to `FunctorT`
 * @param a_tuple the tuple to be processed
 * @param return_v the initial value of the accumulator
 * @param ts the arguments to be passed to `FunctorT`
 */
template <
      typename FunctorT,
      typename ReturnT,
      typename BinaryOperator = std::plus<ReturnT>,
      size_t I = 0,
      typename TupleT,
      typename... Ts>
ReturnT accumulate_tuple(TupleT& a_tuple, ReturnT return_v = ReturnT{}, Ts&&... ts) {
    if constexpr (I < std::tuple_size_v<TupleT>) {
        FunctorT f;
        return_v = BinaryOperator{}(return_v, f.template operator()<I>(a_tuple, ts...));
        return accumulate_tuple<FunctorT, ReturnT, BinaryOperator, I + 1, TupleT>(
              a_tuple,
              return_v,
              std::forward<Ts>(ts)...);
    } else {
        return return_v;
    }
}
} // namespace detail

/**
 * Represents the result of a command line parsing operation.
 *
 * This enum indicates whether parsing was successful, encountered an error,
 * or the user requested help information.
 */
enum class ParseResult {
    Ok,    ///< Parsing completed successfully
    Error, ///< An error occurred during parsing
    Help,  ///< Help was requested (--help or -h flag)
};

/**
 * A flexible, template-based command line argument parser.
 *
 * This class provides a type-safe way to parse command line arguments using
 * compile-time templates. It supports both positional arguments (mandatory)
 * and optional arguments with short and long forms.
 *
 * @tparam ValueTupleT A tuple of objects to process mandatory/positional arguments
 * @tparam ParserTupleT A tuple of objects to process optional arguments
 *
 * @section Requirements
 *
 * ### ValueTupleT Requirements
 * Each element of ValueTupleT should implement:
 * - `void digest(const std::string& argument)` - Process and store the argument
 * - `<type> value() const` - Return the parsed (and possibly converted) value
 * - `bool can_digest() const` - Return if it can still digest more arguments
 * - `std::string name() const` - Return parameter name for usage documentation
 * - `std::string description() const` - Return parameter description for help
 *
 * ### ParserTupleT Requirements
 * Each element should be either BooleanParser or implement:
 * - `void digest(const std::string& argument)` - Process and store the argument
 * - `<type> value() const` - Return the parsed value
 * - `bool is_present() const` - Return if the argument was provided
 * - `std::string name() const` - Return parameter name for documentation
 * - `std::string description() const` - Return parameter description
 * - `char short_option() const` - Return unique short option (not 'h')
 * - `const std::string& long_option() const` - Return unique long option (not "help")
 * - `constexpr static bool IS_BOOLEAN = false;` (for non-boolean parsers)
 *
 * @section Example
 * ```cpp
 * // Create parsers for arguments
 * auto filename_parser = StringParser("filename", "Input file to process");
 * auto verbose_flag = BooleanParser('v', "verbose", "Enable verbose output");
 * auto count_parser = IntParser('c', "count", "number", "Number of iterations");
 *
 * // Create the argument parser
 * auto parser = make_parser(
 *     "My Application - processes files with options",
 *     std::make_tuple(filename_parser),
 *     std::make_tuple(verbose_flag, count_parser),
 *     "For more information, visit: https://example.com"
 * );
 *
 * // Parse command line arguments
 * auto result = parser.parse(argc, argv, std::cerr);
 * if (result == ParseResult::Ok) {
 *     std::cout << "Filename: " << std::get<0>(parser.values()).value() << std::endl;
 *     if (std::get<0>(parser.parsers()).is_present()) {
 *         std::cout << "Verbose mode enabled" << std::endl;
 *     }
 * }
 * ```
 */
template <typename ValueTupleT, typename ParserTupleT>
class ArgParser {
public:
    /// Number of positional arguments in the ValueTupleT
    static constexpr size_t VALUE_SIZE = std::tuple_size_v<ValueTupleT>;
    /// Number of optional arguments in the ParserTupleT
    static constexpr size_t PARSER_SIZE = std::tuple_size_v<ParserTupleT>;

    /**
     * Constructs an `ArgParser` with the specified configuration.
     *
     * @param description Overall description of the application/program
     * @param value_tuple Tuple containing parsers for positional arguments
     * @param parser_tuple Tuple containing parsers for optional arguments
     * @param footer Optional footer text to display after the usage information
     *
     * @throws InvalidArgument if any parser uses reserved option 'h' or "help"
     * @throws InvalidArgument if duplicate short or long options are detected
     */
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

    /**
     * Get access to the parsed positional argument values.
     * @return Const reference to the tuple of positional argument parsers
     */
    const ValueTupleT& values() const { return values_; }

    /**
     * Get access to the parsed optional argument values.
     * @return Const reference to the tuple of optional argument parsers
     */
    const ParserTupleT& parsers() const { return parsers_; }

    /**
     * Generate a formatted usage string for the application.
     *
     * Creates a comprehensive usage message including:
     * - Application description
     * - Command line syntax
     * - Positional parameter descriptions
     * - Optional parameter descriptions
     * - Footer information (if provided)
     *
     * @param executable Name of the executable (typically argv[0])
     * @return Formatted usage string ready for display
     */
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

    /**
     * Parse command line arguments and handle errors/help requests.
     *
     * This is the main entry point for parsing. It processes the command line
     * arguments and automatically handles error reporting and help display.
     *
     * @param argc Number of command line arguments (from main)
     * @param argv Array of command line argument strings (from main)
     * @param err Output stream for error messages and usage information
     * @return ParseResult indicating success, error, or help request
     *
     * @note This function is noexcept and handles all parsing exceptions internally
     */
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
    std::string description_;                 ///< Application description text
    ValueTupleT values_;                      ///< Tuple of positional argument parsers
    ParserTupleT parsers_;                    ///< Tuple of optional argument parsers
    std::string footer_;                      ///< Footer text for usage display
    std::vector<struct option> long_options_; ///< getopt_long compatible option array
    std::string short_options_;               ///< getopt_long compatible option string

    static constexpr const char HELP_[] = "help"; ///< Reserved help option string
    static constexpr const char ERROR_MSG_[] =
          "ERROR processing command line arguments: "; ///< Error message prefix

private: // functions
    /**
     * Internal parsing implementation that may throw exceptions.
     *
     * This function performs the actual parsing work and may throw InvalidArgument
     * exceptions. The public parse() method wraps this and handles exceptions.
     *
     * @param argc Number of command line arguments
     * @param argv Array of command line argument strings
     * @param err Output stream for error messages
     * @return ParseResult indicating the outcome
     * @throws InvalidArgument for various parsing errors
     */
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

    /**
     * Functor to process a parsed command line option.
     *
     * Used with for_each_tuple to find and invoke the appropriate parser
     * for a given short option character.
     */
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

    /**
     * Functor to process positional command line arguments.
     *
     * Used with accumulate_tuple to sequentially process positional arguments
     * and ensure all required arguments are provided.
     */
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

    /**
     * Functor to build formatted usage documentation strings.
     *
     * Creates properly formatted usage text for both positional and optional
     * arguments with consistent indentation and alignment.
     */
    struct BuildUsage {
        /**
         * Format a single line of usage documentation.
         *
         * @param preamble The option syntax (e.g., "-f, --file <filename>")
         * @param description The description of what the option does
         * @return Formatted usage line with proper indentation and alignment
         */
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

    /**
     * Functor to build the command line syntax portion of usage text.
     *
     * Generates the positional argument names for the command line example.
     */
    struct BuildCommandLine {
        template <size_t I>
        std::string operator()(const ValueTupleT& values) {
            auto& value = std::get<I>(values);
            return " " + value.name();
        }
    };

    /**
     * Functor to build getopt_long compatible option structures.
     *
     * Processes parser tuples to create the option arrays needed by getopt_long
     * and validates for duplicate options.
     */
    struct BuildOptions {
        /** Fills the `long_options` and `short_options` from `parsers[I]`. These are filled
         * to be used with `getopt_long`.
         * @param parsers the tuple of parsers to be processed
         * @param long_options the vector of long options to be filled
         * @param short_options the string of short options to be filled
         */
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
                      std::format("Duplicated option: -{}/--{}", short_option, long_option));
            }

            long_options.push_back(opt);
            short_options += short_option;
            if (not parser.IS_BOOLEAN) {
                short_options += ':';
            }
        }
    };

    /**
     * Functor to identify missing required positional arguments.
     *
     * Used to generate error messages when not enough positional arguments
     * were provided on the command line.
     */
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

/**
 * Convenience function to create an `ArgParser` instance.
 *
 * This function template deduces the tuple types automatically, making
 * it easier to create `ArgParser` instances without explicitly specifying
 * the template parameters.
 *
 * @tparam ValueTupleT Deduced type of the positional argument parsers tuple
 * @tparam ParserTupleT Deduced type of the optional argument parsers tuple
 * @param description Overall description of the application
 * @param value_tuple Tuple of positional argument parsers
 * @param parser_tuple Tuple of optional argument parsers
 * @param footer Optional footer text (empty by default)
 * @return Configured `ArgParser` instance ready for use
 *
 * @section Example
 * ```cpp
 * auto parser = make_parser(
 *     "File processor utility",
 *     std::make_tuple(StringParser("input", "Input filename")),
 *     std::make_tuple(BooleanParser('v', "verbose", "Verbose output")),
 *     "Visit https://example.com for more info"
 * );
 * ```
 */
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
