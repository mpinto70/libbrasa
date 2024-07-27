#pragma once

#include <atomic>
#include <list>
#include <mutex>

namespace brasa::thread {

/** Class responsible to hold the \b read-only reference of the current version of the value. */
template <typename T, typename POOL>
class RcuReader {
public:
    /** Creates the RcuReader object with the ref to the current value.
     * @param t the pointer to the current value.
     * @param pool the pointer to the pool that is the owner of the value and will receive it back
     * via release.
     */
    RcuReader(const T* t, const POOL* pool) noexcept : value_(t), pool_(pool) {}
    ~RcuReader() noexcept;
    RcuReader(const RcuReader&) = delete;
    RcuReader(RcuReader&&);
    RcuReader& operator=(const RcuReader&) = delete;
    RcuReader& operator=(RcuReader&&);

    /** Return the read-only reference to the value stored in the pool. */
    const T& value() const noexcept { return *value_; }
    /** Return if the object has a valid value. */
    bool is_valid() const noexcept { return value_ != nullptr && pool_ != nullptr; }

private:
    const T* value_;   ///< the value stored in the pool.
    const POOL* pool_; ///< the pool that owns the value.
};

//-------------------------------------------------------------
// Implementation
//-------------------------------------------------------------

template <typename T, typename POOL>
RcuReader<T, POOL>::~RcuReader() noexcept {
    if (value_ != nullptr && pool_ != nullptr) {
        pool_->release(value_);
    }
}

template <typename T, typename POOL>
RcuReader<T, POOL>::RcuReader(RcuReader&& other) : value_(other.value_),
                                                   pool_(other.pool_) {
    other.value_ = nullptr;
    other.pool_ = nullptr;
}

template <typename T, typename POOL>
RcuReader<T, POOL>& RcuReader<T, POOL>::operator=(RcuReader&& other) {
    value_ = other.value_;
    pool_ = other.pool_;
    other.value_ = nullptr;
    other.pool_ = nullptr;
    return *this;
}

} // namespace brasa::thread
