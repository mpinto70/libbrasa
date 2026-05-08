#pragma once

#include <brasa/thread/RcuReader.h>
#include <brasa/thread/RcuWriter.h>

#include <algorithm>
#include <atomic>
#include <list>
#include <mutex>

namespace brasa::thread {

/**
 * Class that implements a user space RCU (https://en.wikipedia.org/wiki/Read-copy-update) of
 * a \b single value of type T. It is important to assure that the lifetime of this object is longer
 * than the lifetime of any `RcuReader` or `RcuWriter` object.
 */
template <typename T>
class Rcu {
public:
    /**
     * Creates the RCU object with the initial value.
     *
     * @param t the initial value.
     */
    explicit Rcu(T t);
    /**
     * Return the number of available objects.
     *
     * @return the number of nodes currently in the list.
     * @note Must not be called while a `RcuWriter` is active (i.e., between `write()` and
     *       the destruction of the returned `RcuWriter`), as `nodes_mutex_` is held during
     *       that window and `size()` would deadlock.
     */
    size_t size() const noexcept { return nodes_.size(); }
    /**
     * Get a read-only reference to the value.
     * Any number of readers can call this function and the contention is minimal: current_mutex_ is
     * locked to assure proper reference counting. When the `RcuReader` object is destroyed,
     * `release()` is called and `current_mutex_` is locked again to assure proper reference
     * counting.
     *
     * @return an `RcuReader` holding a read-only reference to the current value.
     */
    RcuReader<T, Rcu<T>> read() const noexcept;
    /**
     * Get a read/write reference to change current value.
     * This function locks nodes_mutex_ and will block all other calls to `write()` until the
     * returned `RcuWriter` object is destroyed and `update()` is called, releasing the lock on
     * `nodes_mutex_`. The caller must ensure that the `RcuWriter` object is eventually destroyed
     * so that `update()` is called and `nodes_mutex_` is released.
     *
     * @return an `RcuWriter` holding a read/write reference to a copy of the current value.
     */
    RcuWriter<T, Rcu<T>> write();

private:
    /** The values are stored in this ref-counted structure. */
    struct Node {
        /**
         * Creates a new node with the initial value, and ref-count set to 0.
         *
         * @param value the initial value.
         */
        explicit Node(T value) noexcept : refcount_(0), value_(std::move(value)) {}
        mutable std::atomic<int> refcount_; ///< number of readers.
        T value_;                           ///< the value.
    };

    mutable std::mutex nodes_mutex_;   ///< mutex to protect the nodes list (write mutex).
    std::list<Node> nodes_;            ///< list of nodes with different versions of the value.
    mutable std::mutex current_mutex_; ///< mutex to protect the current node (read mutex).
    const Node* current_;              ///< pointer to the current node (the last one in the list).

    /**
     * Function that is called when the `RcuReader` object is destroyed.
     *
     * @param t the pointer to the value so it can be found to decrease the ref-count.
     */
    void release(const T* t) const noexcept;
    friend class RcuReader<T, Rcu<T>>; ///< It is a friend so it can call `release()`.

    /**
     * Function that is called when the `RcuWriter` object is destroyed.
     * It also performs the cleanup of the nodes that are not being used anymore.
     * Note that this cleanup has to lock `current_mutex_` once to set `current_` to point to the
     * new value. Then it removes unused nodes (refcount == 0). Safe traversal without additional
     * locking because `std::list` erasure does not invalidate other iterators, and any node still
     * referenced has a positive refcount so it won't be erased concurrently.
     *
     * @note `nodes_mutex_` is acquired in `write()` and released at the end of this function.
     */
    void update() noexcept;
    friend class RcuWriter<T, Rcu<T>>; ///< It is a friend so it can call `update()`.
};

//-------------------------------------------------------------
// Implementation
//-------------------------------------------------------------

template <typename T>
Rcu<T>::Rcu(T t) {
    nodes_.emplace_back(std::move(t));
    current_ = &nodes_.back();
}

template <typename T>
RcuReader<T, Rcu<T>> Rcu<T>::read() const noexcept {
    std::lock_guard lock(current_mutex_);
    current_->refcount_ += 1;
    return RcuReader<T, Rcu<T>>(&current_->value_, this);
}

template <typename T>
void Rcu<T>::release(const T* t) const noexcept {
    auto it = std::find_if(nodes_.begin(), nodes_.end(), [t](const Node& node) {
        return &node.value_ == t;
    });
    if (it != nodes_.end()) {
        it->refcount_.fetch_sub(1);
    }
}

template <typename T>
RcuWriter<T, Rcu<T>> Rcu<T>::write() {
    nodes_mutex_.lock();

    try {
        nodes_.emplace_back(nodes_.back().value_);
        return RcuWriter<T, Rcu<T>>(&nodes_.back().value_, this);
    } catch (...) {
        nodes_mutex_.unlock();
        throw;
    }
}

template <typename T>
void Rcu<T>::update() noexcept {
    { // scope update the current pointer
        std::lock_guard lock(current_mutex_);
        current_ = &nodes_.back();
    }

    // remove unreferenced nodes that are not the last one
    for (auto it = nodes_.begin(); std::next(it) != nodes_.end();) {
        if (it->refcount_.load() == 0) {
            it = nodes_.erase(it);
        } else {
            ++it;
        }
    }

    nodes_mutex_.unlock();
}

} // namespace brasa::thread
