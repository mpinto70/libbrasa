#pragma once

#include <memory>
#include <mutex>
#include <stdexcept>
#include <typeinfo>

namespace brasa::pattern {

/** A class that implements Singleton pattern. */
template <typename T>
class Singleton final {
public:
    template <typename... ARGS>
    static T& create_instance(ARGS&&... args);
    static T& instance();
    static void free_instance();
    static bool has_instance();

    Singleton() = delete;
    ~Singleton() = delete;

    Singleton(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton& operator=(Singleton&&) = delete;

private:
    static std::unique_ptr<T> t_;
    static std::mutex mutex_;
};

template <typename T>
std::unique_ptr<T> Singleton<T>::t_;

template <typename T>
std::mutex Singleton<T>::mutex_;

template <typename T>
template <typename... ARGS>
T& Singleton<T>::create_instance(ARGS&&... args) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (t_ != nullptr) {
        using namespace std::string_literals;
        throw std::logic_error("brasa::pattern::Singleton::create_instance already created for "s + typeid(T).name());
    }
    t_ = std::make_unique<T>(std::forward<ARGS>(args)...);
    return *t_;
}

template <typename T>
T& Singleton<T>::instance() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (t_ == nullptr) {
        using namespace std::string_literals;
        throw std::logic_error("brasa::pattern::Singleton::instance not created for "s + typeid(T).name());
    }
    return *t_;
}

template <typename T>
void Singleton<T>::free_instance() {
    std::lock_guard<std::mutex> lock(mutex_);
    t_.reset();
}

template <typename T>
bool Singleton<T>::has_instance() {
    return t_ != nullptr;
}

}
