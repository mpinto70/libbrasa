#pragma once

#include <algorithm>
#include <cstdint>

namespace brasa {
namespace chronometer {

struct Elapsed {
    uint32_t chrono_id;
    uint32_t mark_id;
    uint64_t begin;
    uint64_t end;
} __attribute__((packed));

template<typename NOW_FUNC>
class Chronometer {
public:
    Chronometer(NOW_FUNC&& now, uint32_t id)
        : now_(std::move(now)),
          id_(id),
          begin_(now_()) {
    }
    Chronometer(Chronometer&&) = default;
    Chronometer& operator =(Chronometer&&) = default;
    uint32_t id() const { return id_; }
    Elapsed mark(uint32_t mark_id) const {
        return { id_, mark_id, begin_, now_() };
    }
    void reset() {
        begin_ = now_();
    }
private:
    const NOW_FUNC now_;
    const uint32_t id_;
    uint64_t begin_;

    Chronometer(const Chronometer&) = delete;
    Chronometer& operator =(const Chronometer&) = delete;
};

template<typename NOW_FUNC>
Chronometer<NOW_FUNC> make_chronometer(NOW_FUNC&& func, uint32_t id) {
    return Chronometer<NOW_FUNC>(std::forward<NOW_FUNC>(func), id);
}
}
}
