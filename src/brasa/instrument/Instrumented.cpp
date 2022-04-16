#include <brasa/instrument/Instrumented.h>

#include <algorithm>

namespace brasa {
namespace instrument {
size_t InstrumentedCounter::counts[];
const char* InstrumentedCounter::counter_names[InstrumentedCounter::NUMBER_OPS] = {
    "n",         "dtor", "default ctor", "conv ctor", "conv move ctor", "copy ctor",
    "move ctor", "conv", "assign",       "equal",     "compare",
};

void InstrumentedCounter::initialize(size_t m) {
    std::fill(counts, counts + sizeof(counts) / sizeof(counts[0]), 0);
    counts[n] = m;
}
}
}
