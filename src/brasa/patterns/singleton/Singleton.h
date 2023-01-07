#pragma once

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>

namespace brasa::pattern {

/**
 * A class that implements a thread safe Singleton pattern.
 */
template <typename T>
class Singleton final {
public:
    /**
     * Creates an instance of type T with args passed to constructor.
     * @param args  args passed to constructor
     * @return the singleton
     */
    template <typename... ARGS>
    static T& create_instance(ARGS&&... args);
    /**
     * Creates an instance of type U derived from T with args passed to constructor.
     *
     * @tparam U    type of concrete object derived from T
     * @param args  args passed to constructor
     * @return the singleton
     */
    template <typename U, typename... ARGS>
    static T& create_instance(ARGS&&... args);
    /**
     * Creates an instance of type U derived from T.
     *
     * @tparam U    type of concrete object derived from T
     * @param u     the object of type U
     * @return the singleton
     */
    template <typename U>
    static T& create_instance(std::unique_ptr<U> u);
    /** Return the instance. */
    static T& instance();
    /** Destroy the instance. */
    static void free_instance() noexcept;
    /** Return if there is an instance. */
    static bool has_instance() noexcept;
    /** Return if the instance is of type U or its descendants. */
    template <typename U>
    static bool is_instance_of_type() noexcept;
    /** Return the instance cast to type U
     * @throw std::logic_error if not possible to cast.
     */
    template <typename U>
    static U& instance_of_type();

    Singleton() = delete;
    ~Singleton() = delete;
    Singleton(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton& operator=(Singleton&&) = delete;

private:
    static std::unique_ptr<T> t_;    ///< the instance
    static std::shared_mutex mutex_; ///< the mutex to protect concurrent access

    /** Return a pointer of type U if the instance if of type U or its descendants
     * or nullptr otherwise. */
    template <typename U>
    static U* get_pointer() noexcept;
};

template <typename T>
std::unique_ptr<T> Singleton<T>::t_;

template <typename T>
std::shared_mutex Singleton<T>::mutex_;

template <typename T>
template <typename... ARGS>
T& Singleton<T>::create_instance(ARGS&&... args) {
    return create_instance<T, ARGS...>(std::forward<ARGS>(args)...);
}

template <typename T>
template <typename U, typename... ARGS>
T& Singleton<T>::create_instance(ARGS&&... args) {
    std::unique_lock lock(mutex_);
    if (t_ != nullptr) {
        using namespace std::string_literals;
        throw std::logic_error(
              "brasa::pattern::Singleton::create_instance already created for "s
              + typeid(T).name());
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
        throw std::logic_error(
              "brasa::pattern::Singleton::create_instance already created for "s
              + typeid(T).name());
    }
    t_ = std::move(u);
    return *t_;
}

template <typename T>
T& Singleton<T>::instance() {
    std::shared_lock lock(mutex_);
    if (t_ == nullptr) {
        using namespace std::string_literals;
        throw std::logic_error(
              "brasa::pattern::Singleton::instance not created for "s + typeid(T).name());
    }
    return *t_;
}

template <typename T>
void Singleton<T>::free_instance() noexcept {
    std::unique_lock lock(mutex_);
    t_.reset();
}

template <typename T>
bool Singleton<T>::has_instance() noexcept {
    std::shared_lock lock(mutex_);
    return t_ != nullptr;
}

template <typename T>
template <typename U>
bool Singleton<T>::is_instance_of_type() noexcept {
    std::shared_lock lock(mutex_);
    return get_pointer<U>() != nullptr;
}

template <typename T>
template <typename U>
U& Singleton<T>::instance_of_type() {
    std::shared_lock lock(mutex_);
    U* ptr = get_pointer<U>();
    if (ptr != nullptr) {
        return *ptr;
    } else {
        using namespace std::string_literals;
        const std::string msg = "base::pattern::Singleton<"s + typeid(T).name()
                                + ">::instance_of_type not created for type "s + typeid(U).name();
        throw std::logic_error(msg);
    }
}

template <typename T>
template <typename U>
U* Singleton<T>::get_pointer() noexcept {
    if constexpr (std::is_base_of_v<T, U>) {
        return dynamic_cast<U*>(t_.get());
    } else {
        if constexpr (std::is_same_v<T, U>) {
            return t_.get();
        } else {
            return nullptr;
        }
    }
}

} // namespace brasa::pattern
