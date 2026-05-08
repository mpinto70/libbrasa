#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

namespace brasa::chronus {

/**
 * Records a timing measurement produced by a `Chronometer`.
 *
 * Stores the chronometer and mark identifiers together with the start instant
 * (`begin`) captured at construction time and the end instant (`end`) captured
 * when `Chronometer::mark()` was called.  Both instants are expressed in the
 * tick unit of the `NOW_FUNC` used by the originating chronometer.
 */
struct Elapsed {
    uint32_t chrono_id; ///< identifier of the `Chronometer` that produced this record
    uint32_t mark_id;   ///< caller-supplied identifier for this particular measurement
    uint64_t begin;     ///< tick count captured when the `Chronometer` was created or last reset
    uint64_t end;       ///< tick count captured when `Chronometer::mark()` was called
};

/**
 * A monotonic chronometer that measures elapsed time using a caller-supplied clock function.
 *
 * `NOW_FUNC` must be a callable with signature `T()` where `T` is an arithmetic type
 * representing a tick count (e.g. the functions from `Now.h`).  Each call to `NOW_FUNC`
 * must return a value greater than or equal to the previous call.
 *
 * `ELAPSED` must be an aggregate whose first two members are compatible with `uint32_t`
 * and whose third and fourth members are compatible with the return type of `NOW_FUNC`.
 * The default `ELAPSED` type is `brasa::chronus::Elapsed`.
 *
 * The chronometer records the current tick count on construction (or after `reset()`) as
 * the start instant.  Elapsed time is then obtained via `count()` or `mark()`.
 *
 * Chronometer objects are move-only (copy is disabled).
 *
 * Prefer constructing instances through the `make_chronometer()` factory to benefit from
 * template argument deduction.
 */
template <typename NOW_FUNC, typename ELAPSED>
class Chronometer {
public:
    /**
     * Tick type returned by `NOW_FUNC`, used to store `begin_`
     */
    using TimeT = std::invoke_result_t<NOW_FUNC>;
    /**
     * Constructs the chronometer, capturing the current instant as the start time.
     *
     * @param now Callable used to obtain the current tick count; moved into the object.
     * @param id  Caller-supplied identifier embedded in every `ELAPSED` record produced
     *            by this chronometer.
     */
    Chronometer(NOW_FUNC&& now, uint32_t id) : now_(std::move(now)), id_(id), begin_(now_()) {}
    // no copies, only moves
    Chronometer(const Chronometer&) = delete;
    Chronometer& operator=(const Chronometer&) = delete;
    Chronometer(Chronometer&&) noexcept = default;
    Chronometer& operator=(Chronometer&&) noexcept = default;

    /**
     * Returns the chronometer identifier supplied at construction.
     */
    [[nodiscard]] uint32_t id() const noexcept { return id_; }

    /**
     * Captures the current instant and returns an `ELAPSED` record.
     *
     * The returned record contains `chrono_id` (this chronometer's id), `mark_id`
     * (the caller-supplied value), `begin` (start instant), and `end` (current instant).
     *
     * @param mark_id Caller-supplied identifier for this measurement point.
     * @return An `ELAPSED` struct describing the interval from `begin_` to now.
     */
    [[nodiscard]] ELAPSED mark(uint32_t mark_id) const { return { id_, mark_id, begin_, now_() }; }

    /**
     * Returns the number of ticks elapsed since construction or the last `reset()`.
     */
    [[nodiscard]] uint64_t count() const { return now_() - begin_; }

    /**
     * Resets the start instant to the current tick count.
     *
     * Subsequent calls to `count()` and `mark()` measure from this new start point.
     */
    void reset() noexcept(noexcept(now_())) { begin_ = now_(); }

private:
    NOW_FUNC now_;
    uint32_t id_;
    TimeT begin_;
};

/**
 * Factory function that creates a `Chronometer` with template argument deduction.
 *
 * @tparam ELAPSED   The elapsed-time record type (defaults to `brasa::chronus::Elapsed`).
 * @tparam NOW_FUNC  Deduced from `func`; must satisfy the `Chronometer` requirements.
 * @param func       Callable that returns the current tick count.
 * @param id         Identifier embedded in every `ELAPSED` record produced by the chronometer.
 * @return A `Chronometer<NOW_FUNC, ELAPSED>` whose start instant is set to `func()` at
 *         the moment of construction.
 */
template <typename ELAPSED = Elapsed, typename NOW_FUNC>
Chronometer<NOW_FUNC, ELAPSED> make_chronometer(NOW_FUNC&& func, uint32_t id) {
    return Chronometer<NOW_FUNC, ELAPSED>(std::forward<NOW_FUNC>(func), id);
}
} // namespace brasa::chronus
