#pragma once

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <typeinfo>

namespace brasa::pattern {

/** A class that implements Singleton pattern. */
template <typename T>
class Singleton final {
public:
    template <typename... ARGS>
    static T& create_instance(ARGS&&... args);
    template <typename U, typename... ARGS>
    static T& create_instance(ARGS&&... args);
    template <typename U>
    static T& create_instance(std::unique_ptr<U> u);
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
    static std::shared_mutex mutex_;
};

template <typename T>
std::unique_ptr<T> Singleton<T>::t_;

template <typename T>
std::shared_mutex Singleton<T>::mutex_;

template <typename T>
template <typename... ARGS>
T& Singleton<T>::create_instance(ARGS&&... args) {
    std::unique_lock lock(mutex_);
    if (t_ != nullptr) {
        using namespace std::string_literals;
        throw std::logic_error("brasa::pattern::Singleton::create_instance already created for "s + typeid(T).name());
    }
    t_ = std::make_unique<T>(std::forward<ARGS>(args)...);
    return *t_;
}

template <typename T>
template <typename U, typename... ARGS>
T& Singleton<T>::create_instance(ARGS&&... args) {
    std::unique_lock lock(mutex_);
    if (t_ != nullptr) {
        using namespace std::string_literals;
        throw std::logic_error("brasa::pattern::Singleton::create_instance already created for "s + typeid(T).name());
    }
    t_ = std::make_unique<U>(std::forward<ARGS>(args)...);
    return *t_;
}

template <typename T>
template <typename U>
T& Singleton<T>::create_instance(std::unique_ptr<U> u) {
    std::unique_lock lock(mutex_);
    if (t_ != nullptr) {
        using namespace std::string_literals;
        throw std::logic_error("brasa::pattern::Singleton::create_instance already created for "s + typeid(T).name());
    }
    t_ = std::move(u);
    return *t_;
}

template <typename T>
T& Singleton<T>::instance() {
    std::shared_lock lock(mutex_);
    if (t_ == nullptr) {
        using namespace std::string_literals;
        throw std::logic_error("brasa::pattern::Singleton::instance not created for "s + typeid(T).name());
    }
    return *t_;
}

template <typename T>
void Singleton<T>::free_instance() {
    std::unique_lock lock(mutex_);
    t_.reset();
}

template <typename T>
bool Singleton<T>::has_instance() {
    std::shared_lock lock(mutex_);
    return t_ != nullptr;
}

}
