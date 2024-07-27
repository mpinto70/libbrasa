#pragma once

#include <brasa/argparse/Values.h>

#include <string>

namespace brasa::argparse {

class BaseParser {
public:
    BaseParser(char short_option, std::string long_option, std::string description);

    char short_option() const { return short_option_; }
    const std::string& long_option() const { return long_option_; }
    const std::string& description() const { return description_; }

protected:
    ~BaseParser() noexcept = default;

private:
    char short_option_;
    std::string long_option_;
    std::string description_;
};

class BooleanParser final : public BaseParser {
public:
    constexpr static bool IS_BOOLEAN = true;

    BooleanParser(char short_option, std::string long_option, std::string description);

    void digest() { active_ = true; }
    bool value() const { return active_; }

private:
    bool active_;
};

template <typename CHECKER_FUNCTION_T, typename DIGESTER>
class BaseParameterParser : public BaseParser {
public:
    constexpr static bool IS_BOOLEAN = false;

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

    void digest(const std::string& argument) {
        if (not checker_function_(argument)) {
            throw InvalidArgument("Invalid argument [" + argument + "] for " + long_option());
        }
        present_ = true;
        digester_.digest(argument);
    }
    bool is_present() const { return present_; }
    const std::string& name() const { return name_; }
    const DIGESTER& digester() const { return digester_; }

protected:
    ~BaseParameterParser() noexcept = default;

private:
    bool present_;
    std::string name_;
    CHECKER_FUNCTION_T checker_function_;
    DIGESTER digester_;
};

inline bool not_empty(const std::string& argument) {
    return not argument.empty();
}

template <typename DIGESTER>
class ValueParser : public BaseParameterParser<decltype(&not_empty), DIGESTER> {
public:
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
