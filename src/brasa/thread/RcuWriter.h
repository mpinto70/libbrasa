#pragma once

#include <atomic>
#include <list>
#include <mutex>

namespace brasa::thread {

/** Class responsible to hold the \b read/write reference to the copy of the current value.
 * After destruction, the value of this class will be the new current value.
 */
template <typename T, typename POOL>
class RcuWriter {
public:
    /** Creates the RcuWriter object with the ref to the new current value.
     * @param t the pointer to the new current value.
     * @param pool the pointer to the pool that is the owner of the value and will receive it back
     * via update.
     */
    RcuWriter(T* t, POOL* pool) noexcept : value_(t), pool_(pool) {}
    ~RcuWriter() noexcept;
    RcuWriter(const RcuWriter&) = delete;
    RcuWriter(RcuWriter&&);
    RcuWriter& operator=(const RcuWriter&) = delete;
    RcuWriter& operator=(RcuWriter&&);

    /** Return the read/write reference to the value stored in the pool. */
    T& value() const noexcept { return *value_; }
    /** Return if the object has a valid value. */
    bool is_valid() const noexcept { return value_ != nullptr && pool_ != nullptr; }

private:
    T* value_;   ///< the value stored in the pool.
    POOL* pool_; ///< the pool that owns the value.
};

//-------------------------------------------------------------
// Implementation
//-------------------------------------------------------------

template <typename T, typename POOL>
RcuWriter<T, POOL>::~RcuWriter() noexcept {
    if (value_ != nullptr && pool_ != nullptr) {
        pool_->update();
    }
}

template <typename T, typename POOL>
RcuWriter<T, POOL>::RcuWriter(RcuWriter&& other) : value_(other.value_),
                                                   pool_(other.pool_) {
    other.value_ = nullptr;
    other.pool_ = nullptr;
}

template <typename T, typename POOL>
RcuWriter<T, POOL>& RcuWriter<T, POOL>::operator=(RcuWriter&& other) {
    value_ = other.value_;
    pool_ = other.pool_;
    other.value_ = nullptr;
    other.pool_ = nullptr;
    return *this;
}

} // namespace brasa::thread
