#include "Instrumented.h"
#include <algorithm>

namespace brasa {
namespace instrument {
constexpr size_t InstrumentedCounter::NUMBER_OPS;
size_t InstrumentedCounter::counts[];
const char* InstrumentedCounter::counter_names[NUMBER_OPS] = {
    "n",
    "destruction",
    "default construction",
    "conversion construction",
    "conversion move construction",
    "copy construction",
    "move construction",
    "conversion",
    "assignment",
    "equality",
    "comparison",
};

void InstrumentedCounter::initialize(size_t m) {
    std::fill(counts, counts + NUMBER_OPS, 0);
    counts[n] = m;
}

}
}
