
#include <brasa/buffer/Copy.h>

#include <benchmark/benchmark.h>

#include <algorithm>
#include <numeric>
#include <random>
#include <source_location>
#include <string>

namespace brasa::buffer::impl {
namespace {
constexpr size_t N = 1 << 20;
auto create_random(size_t n, char b, char e) {
    static std::mt19937 eng(std::random_device{}());
    std::uniform_int_distribution<char> dist(b, e);
    std::string out(n, ' ');
    std::generate(out.begin(), out.end(), [&dist]() { return dist(eng); });
    return out;
}

using copy_func_t = void (*)(void*, const void*, size_t);

static void time_function(benchmark::State& state, copy_func_t func) {
    const auto src = create_random(N, 'a', 'z');
    auto dest = create_random(N, 'A', 'Z');
    const auto offset = state.range(0);
    for (auto _ : state) {
        func(dest.data() + offset, src.data() + offset, N - offset);
        benchmark::DoNotOptimize(dest);
    }
}

BENCHMARK_CAPTURE(time_function, CopyBytes, copy_by_byte)
      ->Unit(benchmark::kMicrosecond)
      ->DenseRange(0, 7);

BENCHMARK_CAPTURE(time_function, CopyQWords, copy_by_qword)
      ->Unit(benchmark::kMicrosecond)
      ->DenseRange(0, 7);

BENCHMARK_CAPTURE(time_function, CopyQWordsAligned, copy_by_qword_dest_aligned)
      ->Unit(benchmark::kMicrosecond)
      ->DenseRange(0, 7);

} // namespace
} // namespace brasa::buffer::impl
