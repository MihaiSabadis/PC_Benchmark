#include "refs.h"

// Calibrated on User PC
// Based on 5-run average from run.csv
static const BenchRefs REFS = {
    .pref_integer_mips = 2260.426,
    .pref_float_mflops = 907.005,
    .pref_mem_mbps = 9821.765,
    .pref_aes_mbps = 1672.159,
    .pref_comp_mbps = 724.029,
    .pref_latency_mops = 460.951,
	.pref_disk_mbps = 1562.559
};

const BenchRefs* bench_refs_defaults(void) { return &REFS; }