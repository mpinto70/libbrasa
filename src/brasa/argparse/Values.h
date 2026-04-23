/**
 * Value processing and storage classes for the `brasa::argparse` system.
 *
 * This file provides a hierarchy of classes designed to handle the processing,
 * validation, and storage of parsed command line argument values. These classes
 * work in conjunction with the parser classes to provide type-safe conversion
 * from string arguments to typed values.
 *
 * The design supports both single-value and multi-value arguments with automatic
 * type conversion using standard stream operators. Custom types can be supported
 * by implementing appropriate stream extraction operators.
 */

#pragma once

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace brasa::argparse {

/**
 * Exception thrown when argument parsing encounters invalid input.
 *
 * This exception is used throughout the `argparse` system to signal various
 * types of parsing errors, including:
 * - Invalid argument format or content
 * - Type conversion failures
 * - Constraint violations (e.g., too many arguments for limited parsers)
 *
 * The exception inherits from `std::runtime_error` and provides a descriptive
 * message about what went wrong during parsing.
 *
 * @section Example
 * ```cpp
 * try {
 *     parser.parse(argc, argv, std::cerr);
 * } catch (const InvalidArgument& e) {
 *     std::cerr << "Parsing error: " << e.what() << std::endl;
 * }
 * ```
 */
struct InvalidArgument : std::runtime_error {
    /**
     * Constructs an `InvalidArgument` exception with a descriptive message.
     * @param msg Human-readable description of the parsing error
     */
    explicit InvalidArgument(const std::string& msg) : std::runtime_error(msg) {}
};

/**
 * Utility function to validate argument consumption capability.
 *
 * This function provides a centralized way to check if a value processor
 * can still accept more arguments, and throws an appropriate exception
 * if it cannot. This helps maintain consistent error reporting across
 * all value types.
 *
 * @param can_digest True if the processor can accept another argument
 * @param argument The argument string that would be processed
 * @throws InvalidArgument if `can_digest` is false
 */
void assert_can_digest(bool can_digest, const std::string& argument);

/**
 * Base class providing common functionality for all value processors.
 *
 * This class establishes the fundamental interface that all value processing
 * classes must provide: a name for documentation purposes and a human-readable
 * description for help text generation.
 *
 * The name is typically used in usage messages (e.g., "<filename>") while
 * the description provides detailed explanation of what the parameter represents.
 *
 * @section Thread Safety
 * This class is not thread-safe. Instances should not be shared between threads
 * without external synchronization.
 */
struct BaseValue {
    /**
     * Constructs a BaseValue with name and description.
     *
     * @param name Parameter name for usage documentation (e.g., "filename")
     * @param description Human-readable description for help text
     */
    BaseValue(std::string name, std::string description)
          : name_(std::move(name)),
            description_(std::move(description)) {}

    /**
     * Get the parameter name.
     * @return The parameter name string
     */
    const std::string& name() const { return name_; }

    /**
     * Get the parameter description.
     * @return The parameter description string
     */
    const std::string& description() const { return description_; }

private:
    std::string name_;        ///< Parameter name for documentation
    std::string description_; ///< Human-readable description
};

/**
 * Base class for value processors that accept exactly one argument.
 *
 * This class extends `BaseValue` with state tracking for single-value processors.
 * It maintains a "digested" flag to ensure that single-value processors
 * can only consume one argument from the command line.
 *
 * Classes derived from this base should call `mark_digested()` after successfully
 * processing an argument to prevent further argument consumption.
 */
struct BaseSingleValue : BaseValue {
    /**
     * Constructs a `BaseSingleValue` with name and description.
     *
     * @param name Parameter name for usage documentation
     * @param description Human-readable description for help text
     */
    BaseSingleValue(std::string name, std::string description)
          : BaseValue(std::move(name), std::move(description)),
            digested_(false) {}

    /**
     * Mark this value as having consumed its argument.
     *
     * This should be called by derived classes after successfully
     * processing an argument to prevent further consumption.
     */
    void mark_digested() { digested_ = true; }

    /**
     * Check if this value has already consumed an argument.
     * @return true if an argument has been processed, false otherwise
     */
    bool digested() const { return digested_; }

private:
    bool digested_; ///< True if this value has consumed its argument
};

/**
 * Template class for processing single command line arguments with type conversion.
 *
 * This class handles the conversion of string arguments to typed values using
 * standard stream extraction operators. It supports any type T that has a
 * stream extraction operator (`operator>>`) defined.
 *
 * The class ensures that exactly one argument is consumed and provides
 * type-safe access to the converted value.
 *
 * @tparam T The target type for argument conversion (must support stream extraction)
 *
 * @section Supported Types
 * - All fundamental types (int, double, float, etc.)
 * - Any custom type with `operator>>` defined
 * - String type (handled by specialization)
 *
 * @section Example
 * ```cpp
 * SingleValue<int> port_value("port", "TCP port number");
 * port_value.digest("8080");  // Converts string "8080" to int 8080
 * int port = port_value.value();  // Returns 8080
 *
 * SingleValue<double> ratio_value("ratio", "Scaling ratio");
 * ratio_value.digest("1.5");  // Converts to double 1.5
 * ```
 */
template <typename T>
class SingleValue : private BaseSingleValue {
public:
    /**
     * Constructs a `SingleValue` with name and description.
     *
     * @param name Parameter name for usage documentation
     * @param description Human-readable description for help text
     */
    SingleValue(std::string name, std::string description)
          : BaseSingleValue(std::move(name), std::move(description)),
            t_{} {}

    /**
     * Process a string argument and convert it to type T.
     *
     * Uses standard stream extraction to convert the string argument
     * to the target type. Throws `InvalidArgument` if:
     * - This value has already consumed an argument (`can_digest()` is false)
     * - The string cannot be converted to type T
     *
     * @param argument String argument to process and convert
     * @throws InvalidArgument if argument cannot be processed or converted
     */
    void digest(const std::string& argument) {
        assert_can_digest(can_digest(), argument);
        std::istringstream out(argument);
        out >> t_;
        if (not out) {
            throw InvalidArgument("could not convert value '" + argument + "' of " + name());
        }
        mark_digested();
    }

    /**
     * Check if this value can still accept an argument.
     * @return true if no argument has been processed yet, false otherwise
     */
    bool can_digest() const { return not digested(); }

    // Inherit documentation interface from BaseValue
    using BaseValue::description;
    using BaseValue::name;

    /**
     * Get the converted value.
     * @return Const reference to the stored value of type T
     */
    const T& value() const { return t_; }

private:
    T t_; ///< The stored converted value
};

/**
 * Specialized version of `SingleValue` for string arguments.
 *
 * This specialization handles string arguments without the overhead of
 * stream-based conversion. It directly stores the string argument,
 * providing more efficient processing for string parameters.
 *
 * The specialization maintains the same interface as the generic `SingleValue`
 * but avoids unnecessary string-to-string conversion through streams.
 *
 * @section Example
 * ```cpp
 * SingleValue<std::string> filename("filename", "Input file path");
 * filename.digest("/path/to/file.txt");  // Direct string storage
 * std::string path = filename.value();   // Returns "/path/to/file.txt"
 * ```
 */
template <>
class SingleValue<std::string> : private BaseSingleValue {
public:
    /**
     * Constructs a string `SingleValue` with name and description.
     *
     * @param name Parameter name for usage documentation
     * @param description Human-readable description for help text
     */
    SingleValue(std::string name, std::string description)
          : BaseSingleValue(std::move(name), std::move(description)),
            t_{} {}

    /**
     * Process a string argument by direct storage.
     *
     * Efficiently stores the string argument without conversion overhead.
     * Uses move semantics for optimal performance.
     *
     * @param argument String argument to store
     * @throws InvalidArgument if this value has already consumed an argument
     */
    void digest(std::string argument) {
        assert_can_digest(can_digest(), argument);
        t_.swap(argument);
        mark_digested();
    }

    /**
     * Check if this value can still accept an argument.
     * @return true if no argument has been processed yet, false otherwise
     */
    bool can_digest() const { return not digested(); }

    // Inherit documentation interface from `BaseValue`
    using BaseValue::description;
    using BaseValue::name;

    /**
     * Get the stored string value.
     * @return The stored string
     */
    const std::string& value() const { return t_; }

private:
    std::string t_; ///< The stored string value
};

/**
 * Base class for processing multiple command line arguments of the same type.
 *
 * This class provides the foundation for value processors that can accept
 * multiple arguments and convert them to a collection of typed values.
 * Each argument is processed using `SingleValue` instances, ensuring consistent
 * type conversion and error handling.
 *
 * @tparam T The target type for argument conversion
 *
 * @section Design
 * The class internally maintains a vector of `SingleValue<T>` objects,
 * creating a new one for each argument processed. This ensures that
 * each argument gets proper individual validation and conversion.
 */
template <typename T>
class MultiValueBase : private BaseValue {
public:
    /**
     * Constructs a `MultiValueBase` with name and description.
     *
     * @param name Parameter name for usage documentation
     * @param description Human-readable description for help text
     */
    MultiValueBase(std::string name, std::string description)
          : BaseValue(std::move(name), std::move(description)),
            ts_{} {}

    /**
     * Process a string argument and add it to the collection.
     *
     * Creates a new `SingleValue<T>` instance for the argument and processes
     * it using the same conversion logic as single values. This ensures
     * consistent behavior and error reporting.
     *
     * @param argument String argument to process and convert
     * @throws InvalidArgument if the argument cannot be converted to type T
     */
    void digest(const std::string& argument) {
        std::istringstream out(argument);
        ts_.emplace_back("", "");
        ts_.back().digest(argument);
    }

    // Inherit documentation interface from BaseValue
    using BaseValue::description;
    using BaseValue::name;

    /**
     * Get a specific converted value by index.
     *
     * @param idx Zero-based index of the value to retrieve
     * @return The value at the specified index
     * @note No bounds checking is performed; ensure idx < size()
     */
    const T& value(size_t idx) const { return ts_[idx].value(); }

    /**
     * Get all converted values as a vector.
     *
     * Creates and returns a new vector containing copies of all
     * processed values. This is convenient for accessing all values
     * at once but involves copying.
     *
     * @return All converted values
     */
    std::vector<T> values() const {
        std::vector<T> res;
        std::transform(ts_.begin(), ts_.end(), std::back_inserter(res), [](const auto& t) {
            return t.value();
        });
        return res;
    }

    /**
     * Get the number of arguments processed.
     * @return Number of values in the collection
     */
    size_t size() const noexcept { return ts_.size(); }

private:
    std::vector<SingleValue<T>> ts_; ///< Collection of processed `SingleValue` objects
};

/**
 * Value processor for unlimited multiple arguments of the same type.
 *
 * This class allows processing any number of command line arguments of the
 * same type. It's suitable for parameters that can accept a variable number
 * of values, such as a list of filenames or a series of numbers.
 *
 * The class inherits all functionality from `MultiValueBase` and adds the
 * ability to always accept more arguments (`can_digest()` always returns true).
 *
 * @tparam T The target type for argument conversion
 *
 * @section Example
 * ```cpp
 * MultiValue<std::string> files("files", "Input files to process");
 * files.digest("file1.txt");
 * files.digest("file2.txt");
 * files.digest("file3.txt");
 *
 * auto all_files = files.values();  // Returns vector<string> with 3 files
 * std::string first = files.value(0);  // Returns "file1.txt"
 * ```
 */
template <typename T>
class MultiValue : private MultiValueBase<T> {
public:
    /**
     * Constructs a `MultiValue` with name and description.
     *
     * @param name Parameter name for usage documentation
     * @param description Human-readable description for help text
     */
    MultiValue(std::string name, std::string description)
          : MultiValueBase<T>(std::move(name), std::move(description)) {}

    /**
     * Check if this value can accept more arguments.
     * @return Always true (can accept unlimited arguments)
     */
    constexpr static bool can_digest() { return true; }

    // Inherit all processing and access functionality from `MultiValueBase`
    using MultiValueBase<T>::digest;
    using MultiValueBase<T>::name;
    using MultiValueBase<T>::description;
    using MultiValueBase<T>::value;
    using MultiValueBase<T>::values;
};

/**
 * Value processor for a limited number of multiple arguments.
 *
 * This class allows processing up to `N` command line arguments of the same type.
 * It's useful when you want to accept multiple values but need to enforce
 * an upper limit, such as "up to 3 backup files" or "at most 5 retry counts".
 *
 * The class provides the same interface as `MultiValue` but adds bounds checking
 * to prevent processing more than `N` arguments.
 *
 * @tparam N Maximum number of arguments that can be processed
 * @tparam T The target type for argument conversion
 *
 * @section Example
 * ```cpp
 * LimitedMultiValue<3, std::string> backups("backups", "Up to 3 backup files");
 * backups.digest("backup1.dat");  // OK
 * backups.digest("backup2.dat");  // OK
 * backups.digest("backup3.dat");  // OK
 * // backups.digest("backup4.dat");  // Would throw InvalidArgument
 *
 * bool can_more = backups.can_digest();  // Returns false
 * auto all_backups = backups.values();   // Returns vector with 3 files
 * ```
 */
template <size_t N, typename T>
class LimitedMultiValue : private MultiValueBase<T> {
public:
    /**
     * Constructs a `LimitedMultiValue` with name and description.
     *
     * @param name Parameter name for usage documentation
     * @param description Human-readable description for help text
     */
    LimitedMultiValue(std::string name, std::string description)
          : MultiValueBase<T>(std::move(name), std::move(description)) {}

    /**
     * Check if this value can accept more arguments.
     * @return true if fewer than `N` arguments have been processed, false otherwise
     */
    bool can_digest() { return MultiValueBase<T>::size() < N; }

    /**
     * Process a string argument with bounds checking.
     *
     * Processes the argument only if the limit `N` has not been reached.
     * Throws `InvalidArgument` if attempting to process more than `N` arguments.
     *
     * @param argument String argument to process and convert
     * @throws InvalidArgument if `N` arguments have already been processed
     * @throws InvalidArgument if the argument cannot be converted to type `T`
     */
    void digest(const std::string& argument) {
        assert_can_digest(can_digest(), argument);
        MultiValueBase<T>::digest(argument);
    }

    // Inherit access functionality from MultiValueBase
    using MultiValueBase<T>::name;
    using MultiValueBase<T>::description;
    using MultiValueBase<T>::value;
    using MultiValueBase<T>::values;
};

} // namespace brasa::argparse
