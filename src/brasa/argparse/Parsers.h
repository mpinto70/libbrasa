/**
 * Command line argument parser implementations for the `brasa::argparse` system.
 *
 * This file provides a hierarchy of parser classes designed to handle different types
 * of command line arguments. The parsers are used by `ArgParser` to process both
 * optional flags/parameters and positional arguments with type safety and validation.
 *
 * The design follows a template-based approach that allows for compile-time type
 * checking and custom validation functions while maintaining a clean, extensible
 * interface for different argument types.
 */

#pragma once

#include <brasa/argparse/Values.h>

#include <string>

namespace brasa::argparse {

/**
 * Base class providing common functionality for all command line argument parsers.
 *
 * This abstract base class defines the interface that all argument parsers must implement.
 * It handles the storage and access of the option identifiers (short and long forms)
 * and the human-readable description used in help text generation.
 *
 * All concrete parser classes should inherit from this class either directly or
 * through one of the specialized base classes like BaseParameterParser.
 *
 * @section Thread Safety
 * This class is not thread-safe. Instances should not be shared between threads
 * without external synchronization.
 */
class BaseParser {
public:
    /**
     * Constructs a BaseParser with the specified option identifiers and description.
     *
     * @param short_option Single character identifier for the short form (e.g., 'v' for `-v`)
     * @param long_option String identifier for the long form (e.g., "verbose" for `--verbose`)
     * @param description Human-readable description for help text generation
     *
     * @note The short_option must be unique across all parsers in an `ArgParser` instance.
     *       The long_option must also be unique and cannot be `help` (reserved).
     */
    BaseParser(char short_option, std::string long_option, std::string description);

    /**
     * Get the short option character.
     * @return The single character used for the short form of this option
     */
    char short_option() const { return short_option_; }

    /**
     * Get the long option string.
     * @return The string used for the long form of this option
     */
    const std::string& long_option() const { return long_option_; }

    /**
     * Get the description text.
     * @return The human-readable description of this option
     */
    const std::string& description() const { return description_; }

protected:
    /**
     * Protected destructor to enforce proper inheritance.
     *
     * This destructor is protected to ensure that BaseParser instances
     * can only be destroyed through derived class destructors, maintaining
     * proper object lifecycle management in the inheritance hierarchy.
     */
    ~BaseParser() noexcept = default;

private:
    char short_option_;       ///< Single character identifier for short option form
    std::string long_option_; ///< String identifier for long option form
    std::string description_; ///< Human-readable description for help text
};

/**
 * Parser for boolean/flag command line arguments.
 *
 * This parser handles command line options that don't take parameters and simply
 * indicate presence or absence (like `--verbose` or `-v`). The value is false by
 * default and becomes true when the flag is encountered on the command line.
 *
 * Boolean parsers are detected by the `ArgParser` through the `IS_BOOLEAN` constant
 * and are handled specially during parsing (no argument consumption).
 *
 * @section Example
 * ```cpp
 * BooleanParser verbose_flag('v', "verbose", "Enable verbose output");
 * // Usage: program -v or program --verbose
 *
 * // After parsing:
 * if (verbose_flag.value()) {
 *     std::cout << "Verbose mode is enabled" << std::endl;
 * }
 * ```
 */
class BooleanParser final : public BaseParser {
public:
    /// Compile-time constant indicating this parser handles boolean arguments
    constexpr static bool IS_BOOLEAN = true;

    /**
     * Constructs a BooleanParser with the specified option identifiers and description.
     *
     * @param short_option Single character for the short form (e.g., 'v' for `-v`)
     * @param long_option String for the long form (e.g., "verbose" for `--verbose`)
     * @param description Human-readable description for help text
     */
    BooleanParser(char short_option, std::string long_option, std::string description);

    /**
     * Process the boolean flag (set it to true).
     *
     * This method is called when the flag is encountered on the command line.
     * It sets the internal state to true, indicating the flag was present.
     */
    void digest() { active_ = true; }

    /**
     * Get the current boolean value.
     * @return true if the flag was present on the command line, false otherwise
     */
    bool value() const { return active_; }

private:
    bool active_; ///< Internal state: true if flag was encountered, false otherwise
};

/**
 * Template base class for parsers that accept string parameters with validation.
 *
 * This class provides a foundation for creating parsers that consume command line
 * arguments as string values, validate them using a checker function, and then
 * process them using a digester object.
 *
 * The template design allows for compile-time specification of both the validation
 * function type and the digester type, enabling type-safe and efficient parsing
 * with custom validation logic.
 *
 * @tparam CHECKER_FUNCTION_T Type of the validation function (usually function pointer)
 * @tparam DIGESTER Type of the object that processes and stores the parsed value
 *
 * @section Usage Pattern
 * ```cpp
 * // Custom validation function
 * bool validate_positive_int(const std::string& arg) {
 *     try {
 *         int val = std::stoi(arg);
 *         return val > 0;
 *     } catch (...) {
 *         return false;
 *     }
 * }
 *
 * // Create a parser with custom validation
 * BaseParameterParser<decltype(&validate_positive_int), IntDigester> parser(
 *     'c', "count", "number", "Positive integer count", validate_positive_int
 * );
 * ```
 */
template <typename CHECKER_FUNCTION_T, typename DIGESTER>
class BaseParameterParser : public BaseParser {
public:
    /// Compile-time constant indicating this parser takes string parameters
    constexpr static bool IS_BOOLEAN = false;

    /**
     * Constructs a BaseParameterParser with validation and processing capabilities.
     *
     * @param short_option Single character for the short form
     * @param long_option String for the long form
     * @param name Parameter name used in usage documentation (e.g., "filename")
     * @param description Human-readable description for help text
     * @param checker_function Function to validate argument strings before processing
     */
    BaseParameterParser(
          char short_option,
          std::string long_option,
          std::string name,
          std::string description,
          CHECKER_FUNCTION_T checker_function)
          : BaseParser(short_option, std::move(long_option), std::move(description)),
            present_(false),
            name_(std::move(name)),
            checker_function_(checker_function),
            digester_("", "") {}

    /**
     * Process and validate a command line argument.
     *
     * This method first validates the argument using the checker function,
     * then processes it using the digester if validation succeeds.
     *
     * @param argument The string argument from the command line
     * @throws InvalidArgument if the checker function rejects the argument
     */
    void digest(const std::string& argument) {
        if (not checker_function_(argument)) {
            throw InvalidArgument("Invalid argument [" + argument + "] for " + long_option());
        }
        present_ = true;
        digester_.digest(argument);
    }

    /**
     * Check if this parameter was provided on the command line.
     * @return true if the parameter was encountered and processed, false otherwise
     */
    bool is_present() const { return present_; }

    /**
     * Get the parameter name for documentation purposes.
     * @return The parameter name string
     */
    const std::string& name() const { return name_; }

    /**
     * Get access to the internal digester object.
     * @return The digester that processed the argument
     */
    const DIGESTER& digester() const { return digester_; }

protected:
    /**
     * Protected destructor to enforce proper inheritance.
     */
    ~BaseParameterParser() noexcept = default;

private:
    bool present_;                        ///< True if parameter was encountered on command line
    std::string name_;                    ///< Parameter name for documentation (e.g., "filename")
    CHECKER_FUNCTION_T checker_function_; ///< Validation function for arguments
    DIGESTER digester_;                   ///< Object that processes and stores the parsed value
};

/**
 * Simple validation function that checks for non-empty strings.
 *
 * This utility function provides basic validation for command line arguments
 * by ensuring they are not empty strings. It's commonly used as the default
 * validation for parsers that accept any non-empty string value.
 *
 * @param argument The string argument to validate
 * @return true if the argument is not empty, false if it is empty
 *
 * @section Example
 * ```cpp
 * // Usage in custom parsers
 * if (not_empty(user_input)) {
 *     // Process the non-empty input
 *     process_argument(user_input);
 * }
 * ```
 */
inline bool not_empty(const std::string& argument) {
    return not argument.empty();
}

/**
 * Convenient parser template for string parameters with non-empty validation.
 *
 * This template class provides a simplified interface for creating parsers that
 * accept string parameters and only require basic "not empty" validation.
 * It inherits from `BaseParameterParser` and automatically configures it with
 * the not_empty validation function.
 *
 * This is the most commonly used parser type for general string parameters
 * like filenames, names, URLs, etc., where any non-empty string is acceptable.
 *
 * @tparam DIGESTER Type of the object that processes and stores the parsed value
 *
 * @section Example
 * ```cpp
 * // Create a simple string parameter parser
 * ValueParser<StringDigester> filename_parser(
 *     'f', "file", "filename", "Input file to process"
 * );
 *
 * // Usage: program --file myfile.txt or program -f myfile.txt
 * // After parsing:
 * std::string filename = filename_parser.digester().value();
 * ```
 *
 * @see BaseParameterParser for the full interface
 * @see not_empty for the validation function used
 */
template <typename DIGESTER>
class ValueParser : public BaseParameterParser<decltype(&not_empty), DIGESTER> {
public:
    /**
     * Constructs a ValueParser with non-empty string validation.
     *
     * @param short_option Single character for the short form
     * @param long_option String for the long form
     * @param name Parameter name for documentation (shown in usage as <name>)
     * @param description Human-readable description for help text
     */
    ValueParser(
          char short_option,
          std::string long_option,
          std::string name,
          std::string description)
          : BaseParameterParser<decltype(&not_empty), DIGESTER>(
                  short_option,
                  std::move(long_option),
                  std::move(name),
                  std::move(description),
                  not_empty) {}
};

} // namespace brasa::argparse
