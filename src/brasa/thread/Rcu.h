#pragma once

#include <atomic>
#include <list>
#include <mutex>

namespace brasa::thread {

template <typename T>
class Rcu;

template <typename T>
class RcuReader {
public:
    ~RcuReader() noexcept;
    RcuReader(const RcuReader&) = delete;
    RcuReader(RcuReader&&);
    RcuReader& operator=(const RcuReader&) = delete;
    RcuReader& operator=(RcuReader&&);

    const T& value() const noexcept { return *value_; }

private:
    const T* value_;
    const Rcu<T>* pool_;

    RcuReader(const T* t, const Rcu<T>* pool) noexcept : value_(t), pool_(pool) {}
    friend class Rcu<T>;
};

template <typename T>
class RcuWriter {
public:
    ~RcuWriter() noexcept;
    RcuWriter(const RcuWriter&) = delete;
    RcuWriter(RcuWriter&&);
    RcuWriter& operator=(const RcuWriter&) = delete;
    RcuWriter& operator=(RcuWriter&&);

    T& value() const noexcept { return *value_; }

private:
    T* value_;
    Rcu<T>* pool_;

    RcuWriter(T* t, Rcu<T>* pool) noexcept : value_(t), pool_(pool) {}
    friend class Rcu<T>;
};

template <typename T>
class Rcu {
public:
    explicit Rcu(T t);
    size_t size() const noexcept { return nodes_.size(); }
    RcuReader<T> read() const noexcept;
    RcuWriter<T> write() noexcept;

private:
    struct Node {
        Node(T value) noexcept : refcount_(0), value_(std::move(value)) {}
        mutable int refcount_;
        T value_;
    };

    std::mutex writer_mutex_;
    std::list<Node> nodes_;
    mutable std::mutex reader_mutex_;
    const Node* current_;

    void release(const T* t) const noexcept;
    friend class RcuReader<T>;

    void update() noexcept;
    friend class RcuWriter<T>;
};

//-------------------------------------------------------------
// Implementation
//-------------------------------------------------------------

// RcuReader ----------------------------------------------------

template <typename T>
RcuReader<T>::~RcuReader() noexcept {
    if (value_ != nullptr && pool_ != nullptr) {
        pool_->release(value_);
    }
}

template <typename T>
RcuReader<T>::RcuReader(RcuReader&& other) : value_(other.value_),
                                             pool_(other.pool_) {
    other.value_ = nullptr;
    other.pool_ = nullptr;
}

template <typename T>
RcuReader<T>& RcuReader<T>::operator=(RcuReader&& other) {
    value_ = other.value_;
    pool_ = other.pool_;
    other.value_ = nullptr;
    other.pool_ = nullptr;
    return *this;
}

// RcuWriter --------------------------------------------------

template <typename T>
RcuWriter<T>::~RcuWriter() noexcept {
    if (value_ != nullptr && pool_ != nullptr) {
        pool_->update();
    }
}

template <typename T>
RcuWriter<T>::RcuWriter(RcuWriter&& other) : value_(other.value_),
                                             pool_(other.pool_) {
    other.value_ = nullptr;
    other.pool_ = nullptr;
}

template <typename T>
RcuWriter<T>& RcuWriter<T>::operator=(RcuWriter&& other) {
    value_ = other.value_;
    pool_ = other.pool_;
    other.value_ = nullptr;
    other.pool_ = nullptr;
    return *this;
}

// Rcu --------------------------------------------------------

template <typename T>
Rcu<T>::Rcu(T t) {
    nodes_.emplace_back(std::move(t));
    current_ = &nodes_.back();
}

template <typename T>
RcuReader<T> Rcu<T>::read() const noexcept {
    std::lock_guard lock(reader_mutex_);
    current_->refcount_ += 1;
    return RcuReader<T>(&current_->value_, this);
}

template <typename T>
void Rcu<T>::release(const T* t) const noexcept {
    auto it = std::find_if(nodes_.begin(), nodes_.end(), [t](const Node& node) {
        return &node.value_ == t;
    });
    if (it != nodes_.end()) {
        std::lock_guard lock(reader_mutex_);
        it->refcount_ -= 1;
    }
}

template <typename T>
RcuWriter<T> Rcu<T>::write() noexcept {
    writer_mutex_.lock();

    nodes_.push_back(nodes_.back());
    return RcuWriter<T>(&nodes_.back().value_, this);
}

template <typename T>
void Rcu<T>::update() noexcept {
    { // scope for reader lock
        std::lock_guard lock(reader_mutex_);
        current_ = &nodes_.back();
    }

    // remove unreferenced nodes that are not the last one
    for (auto it = nodes_.begin(); std::next(it) != nodes_.end();) {
        if (it->refcount_ == 0) {
            std::lock_guard lock(reader_mutex_);
            if (it->refcount_ == 0) {
                it = nodes_.erase(it);
                continue;
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }

    writer_mutex_.unlock();
}

} // namespace brasa::thread