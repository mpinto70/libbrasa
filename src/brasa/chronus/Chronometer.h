#pragma once

#include <algorithm>
#include <cstdint>

namespace brasa {
namespace chronus {

struct Elapsed {
    uint32_t chrono_id;
    uint32_t mark_id;
    uint64_t begin;
    uint64_t end;
} __attribute__((packed));

template <typename NOW_FUNC>
class Chronometer {
public:
    Chronometer(NOW_FUNC now, uint32_t id)
          : now_(std::move(now)),
            id_(id),
            begin_(now_()) {
    }
    // no copies, only moves
    Chronometer(const Chronometer&) = delete;
    Chronometer& operator=(const Chronometer&) = delete;
    Chronometer(Chronometer&&) noexcept = default;
    Chronometer& operator=(Chronometer&&) noexcept = default;

    [[nodiscard]] uint32_t id() const noexcept { return id_; }

    [[nodiscard]] Elapsed mark(uint32_t mark_id) const {
        return { id_, mark_id, begin_, now_() };
    }

    void reset() noexcept {
        begin_ = now_();
    }

private:
    const NOW_FUNC now_;
    const uint32_t id_;
    uint64_t begin_;
};

template <typename NOW_FUNC>
Chronometer<NOW_FUNC> make_chronometer(NOW_FUNC func, uint32_t id) {
    return Chronometer<NOW_FUNC>(std::forward<NOW_FUNC>(func), id);
}
}
}
