#pragma once

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace brasa::argparse {

struct InvalidArgument : std::runtime_error {
    explicit InvalidArgument(const std::string& msg) : std::runtime_error(msg) {}
};

void assert_can_digest(bool can_digest, const std::string& argument);

struct BaseValue {
    BaseValue(std::string name, std::string description)
          : name_(std::move(name)),
            description_(std::move(description)) {}

    const std::string& name() const { return name_; }
    const std::string& description() const { return description_; }

private:
    std::string name_;
    std::string description_;
};

struct BaseSingleValue : BaseValue {
    BaseSingleValue(std::string name, std::string description)
          : BaseValue(std::move(name), std::move(description)),
            digested_(false) {}

    void mark_digested() { digested_ = true; }
    bool digested() const { return digested_; }

private:
    bool digested_;
};

template <typename T>
class SingleValue : private BaseSingleValue {
public:
    SingleValue(std::string name, std::string description)
          : BaseSingleValue(std::move(name), std::move(description)),
            t_{} {}

    void digest(const std::string& argument) {
        assert_can_digest(can_digest(), argument);
        std::istringstream out(argument);
        out >> t_;
        if (not out) {
            throw InvalidArgument("could not convert value '" + argument + "' of " + name());
        }
        mark_digested();
    }

    bool can_digest() const { return not digested(); }
    using BaseValue::description;
    using BaseValue::name;

    const T& value() const { return t_; }

private:
    T t_;
};

template <>
class SingleValue<std::string> : private BaseSingleValue {
public:
    SingleValue(std::string name, std::string description)
          : BaseSingleValue(std::move(name), std::move(description)),
            t_{} {}

    void digest(std::string argument) {
        assert_can_digest(can_digest(), argument);
        t_.swap(argument);
        mark_digested();
    }

    bool can_digest() const { return not digested(); }
    using BaseValue::description;
    using BaseValue::name;

    const std::string& value() const { return t_; }

private:
    std::string t_;
};

template <typename T>
class MultiValueBase : private BaseValue {
public:
    MultiValueBase(std::string name, std::string description)
          : BaseValue(std::move(name), std::move(description)),
            ts_{} {}

    void digest(const std::string& argument) {
        std::istringstream out(argument);
        ts_.emplace_back("", "");
        ts_.back().digest(argument);
    }

    using BaseValue::description;
    using BaseValue::name;

    const T& value(size_t idx) const { return ts_[idx].value(); }
    std::vector<T> values() const {
        std::vector<T> res;
        std::transform(ts_.begin(), ts_.end(), std::back_inserter(res), [](const auto& t) {
            return t.value();
        });
        return res;
    }

    size_t size() const noexcept { return ts_.size(); }

private:
    std::vector<SingleValue<T>> ts_;
};

template <typename T>
class MultiValue : private MultiValueBase<T> {
public:
    MultiValue(std::string name, std::string description)
          : MultiValueBase<T>(std::move(name), std::move(description)) {}

    constexpr static bool can_digest() { return true; }

    using MultiValueBase<T>::digest;
    using MultiValueBase<T>::name;
    using MultiValueBase<T>::description;
    using MultiValueBase<T>::value;
    using MultiValueBase<T>::values;
};

template <size_t N, typename T>
class LimitedMultiValue : private MultiValueBase<T> {
public:
    LimitedMultiValue(std::string name, std::string description)
          : MultiValueBase<T>(std::move(name), std::move(description)) {}

    bool can_digest() { return MultiValueBase<T>::size() < N; }

    void digest(const std::string& argument) {
        assert_can_digest(can_digest(), argument);
        MultiValueBase<T>::digest(argument);
    }
    using MultiValueBase<T>::name;
    using MultiValueBase<T>::description;
    using MultiValueBase<T>::value;
    using MultiValueBase<T>::values;
};

}
