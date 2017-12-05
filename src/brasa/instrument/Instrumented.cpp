#include "Instrumented.h"
#include <algorithm>

namespace brasa {
namespace instrument {
constexpr size_t InstrumentedCounter::NUMBER_OPS;
size_t InstrumentedCounter::counts[];
const char* InstrumentedCounter::counter_names[NUMBER_OPS] = {
    "n",
    "dtor",
    "default ctor",
    "conv ctor",
    "conv move ctor",
    "copy ctor",
    "move ctor",
    "conv",
    "assign",
    "equal",
    "compare",
};

void InstrumentedCounter::initialize(size_t m) {
    std::fill(counts, counts + NUMBER_OPS, 0);
    counts[n] = m;
}

}
}
