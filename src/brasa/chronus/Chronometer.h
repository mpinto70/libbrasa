#pragma once

#include <algorithm>
#include <cstdint>

namespace brasa {
namespace chronus {

/** A struct to register the instants in time from `Chronometer` objects. */
struct Elapsed {
    uint32_t chrono_id; ///< chronometer identifier
    uint32_t mark_id;   ///< instant identifier
    uint64_t begin;     ///< start of timing
    uint64_t end;       ///< instant timing
};

/** A class to mark serve as a chronometer
 * NOW_FUNC is a function or functor that returns a suitable representation of current instant in time
 * ELAPSED is a structure in which the two first members are compatible with uint32_t and the third 
 *      and fourth members compatible with the return type of `NOW_FUNC`.
 */
template <typename NOW_FUNC, typename ELAPSED>
class Chronometer {
public:
    using TimeT = std::invoke_result_t<NOW_FUNC>; ///< the return of NOW_FUNC is stored in `begin_`

    /// Creates the object registering its id and the `NOW_FUNC`
    Chronometer(NOW_FUNC&& now, uint32_t id)
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

    /// Marks the current instant and returns an `ELAPSED` struct
    [[nodiscard]] ELAPSED mark(uint32_t mark_id) const {
        return { id_, mark_id, begin_, now_() };
    }

    /// Return the tick count since time began
    [[nodiscard]] uint64_t count() const {
        return now_() - begin_;
    }

    /// Restarts the chronometer
    void reset() noexcept {
        begin_ = now_();
    }

private:
    const NOW_FUNC now_;
    const uint32_t id_;
    TimeT begin_;
};

template <typename ELAPSED = Elapsed, typename NOW_FUNC>
Chronometer<NOW_FUNC, ELAPSED> make_chronometer(NOW_FUNC&& func, uint32_t id) {
    return Chronometer<NOW_FUNC, ELAPSED>(std::forward<NOW_FUNC>(func), id);
}
}
}
