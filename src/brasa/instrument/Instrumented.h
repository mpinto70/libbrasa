#pragma once
// based in work from Alex Stepanov
// https://www.youtube.com/watch?v=aIHAEYyoTUc&list=PLHxtyCq_WDLXryyw91lahwdtpZsmo4BGD

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace brasa::instrument {

/**
 * Per-type counters and metadata for instrumented type operations.
 *
 * This struct holds static arrays that count how many times each kind of operation
 * has been performed on `Instrumented<T>` instances. Each template instantiation
 * `InstrumentedCounter<T>` maintains independent counters, so multiple types can
 * be instrumented simultaneously without interference.
 *
 * Typical usage:
 * @code
 *   InstrumentedCounter<int>::initialize(vec.size());
 *   std::sort(vec.begin(), vec.end());
 *   std::cout << InstrumentedCounter<int>::counts[InstrumentedCounter<int>::comparison];
 * @endcode
 *
 * @tparam T the value type whose operations are being counted.
 * @note Counters use `std::atomic<size_t>`, so individual increments are thread-safe.
 *       However, `initialize()` must only be called when no `Instrumented<T>` operations
 *       are in progress, as the reset is not performed atomically as a whole.
 */
template <typename T>
struct InstrumentedCounter {
    /**
     * Enumeration of all tracked operation types.
     */
    enum operations {
        n, ///< Number of elements under study (set by initialize()).
        destruction,
        default_construction,
        conversion_construction,
        conversion_move_construction,
        copy_construction,
        move_construction,
        conversion,
        assignment,
        move_assignment,
        equality,
        comparison,
        NUMBER_OPS ///< Sentinel --€” must be last.
    };
    /**
     * Counters indexed by `operations`. Reset and seeded by `initialize()`.
     */
    inline static std::atomic<size_t> counts[NUMBER_OPS] = {};
    /**
     * Human-readable names for each operation, indexed by `operations`.
     */
    inline static constexpr const char* counter_names[NUMBER_OPS] = {
        "n",         "dtor", "default ctor", "conv ctor",   "conv move ctor", "copy ctor",
        "move ctor", "conv", "assign",       "move assign", "equal",          "compare",
    };
    /**
     * Reset all counters to zero and record the number of elements under study.
     * @param m number of elements (stored in `counts[n]`)
     * @note Must only be called when no concurrent `Instrumented<T>` operations are
     *       in progress. The reset across all counters is not atomic as a whole.
     */
    static void initialize(size_t m) {
        std::fill_n(counts, NUMBER_OPS, 0);
        counts[n] = m;
    }
};

/**
 * Wrapper that instruments all fundamental operations of type `T`.
 *
 * Every construction, destruction, assignment, conversion, equality check, and
 * comparison performed through this wrapper increments the corresponding counter
 * in `InstrumentedCounter<T>::counts`. This allows algorithmic analysis of how many
 * of each operation a given algorithm performs on its elements.
 *
 * Each `Instrumented<T>` specialization has its own independent counters via
 * `InstrumentedCounter<T>`, so multiple types can be instrumented simultaneously.
 *
 * @tparam T the wrapped value type. Must be at least default-constructible.
 *           Equality and ordering operators are conditionally provided based on
 *           whether `T` satisfies `std::equality_comparable` and `std::totally_ordered`.
 *
 * @note Counter increments are thread-safe (via `std::atomic`), but `initialize()`
 *       must not be called concurrently with any `Instrumented<T>` operations.
 */
template <typename T>
class Instrumented : private InstrumentedCounter<T> {
    using Counter = InstrumentedCounter<T>;

public:
    using value_type = T;
    /**
     * The wrapped value. Public for convenient access in tests and demos.
     * Direct mutations bypass instrumentation.
     */
    T value;

    // conversions
    /**
     * Constructs from a const lvalue of `T`, counting a conversion construction.
     */
    explicit Instrumented(const T& x) : value(x) {
        ++Counter::counts[Counter::conversion_construction];
    }
    /**
     * Constructs from an rvalue of `T`, counting a conversion move construction.
     */
    explicit Instrumented(T&& x) : value(std::move(x)) {
        ++Counter::counts[Counter::conversion_move_construction];
    }
    /**
     * Converts to `T`, counting a conversion.
     */
    explicit operator T() const {
        ++Counter::counts[Counter::conversion];
        return value;
    }

    // semi-regular operations
    /**
     * Copy constructor --€” counts a copy construction.
     */
    Instrumented(const Instrumented& x) noexcept(std::is_nothrow_copy_constructible_v<T>)
          : value(x.value) {
        ++Counter::counts[Counter::copy_construction];
    }
    /**
     * Move constructor --€” counts a move construction.
     */
    Instrumented(Instrumented&& x) noexcept(std::is_nothrow_move_constructible_v<T>)
          : value(std::move(x.value)) {
        ++Counter::counts[Counter::move_construction];
    }
    /**
     * Default constructor --€” counts a default construction.
     */
    Instrumented() noexcept(std::is_nothrow_default_constructible_v<T>) {
        ++Counter::counts[Counter::default_construction];
    }
    /**
     * Destructor --€” counts a destruction.
     */
    ~Instrumented() noexcept(std::is_nothrow_destructible_v<T>) {
        ++Counter::counts[Counter::destruction];
    }
    /**
     * Copy assignment --€” counts an assignment.
     */
    Instrumented& operator=(const Instrumented& y) noexcept(std::is_nothrow_copy_assignable_v<T>) {
        ++Counter::counts[Counter::assignment];
        value = y.value;
        return *this;
    }
    /**
     * Move assignment --€” counts a move assignment.
     */
    Instrumented& operator=(Instrumented&& y) noexcept(std::is_nothrow_move_assignable_v<T>) {
        ++Counter::counts[Counter::move_assignment];
        value = std::move(y.value);
        return *this;
    }

    // regular operations
    /**
     * Equality comparison --€” counts an equality check. `operator!=` is synthesized.
     */
    friend bool operator==(const Instrumented& x, const Instrumented& y) noexcept(
          noexcept(x.value == y.value))
        requires std::equality_comparable<T>
    {
        ++Counter::counts[Counter::equality];
        return x.value == y.value;
    }

    // totally ordered operations
    /**
     * Three-way comparison --€” counts a comparison. All relational operators are synthesized.
     */
    friend auto operator<=>(const Instrumented& x, const Instrumented& y) noexcept(
          noexcept(x.value <=> y.value))
        requires std::totally_ordered<T>
    {
        ++Counter::counts[Counter::comparison];
        return x.value <=> y.value;
    }
};
} // namespace brasa::instrument
