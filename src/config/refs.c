#include "refs.h"

// Calibrated on User PC
// Based on 5-run average from run.csv
static const BenchRefs REFS = {
    .pref_integer_mips = 2255.066,
    .pref_float_mflops = 904.596,
    .pref_mem_mbps = 10195.801,
    .pref_aes_mbps = 1682.464,
    .pref_comp_mbps = 721.390,
    .pref_latency_mops = 468.004
};

const BenchRefs* bench_refs_defaults(void) { return &REFS; }