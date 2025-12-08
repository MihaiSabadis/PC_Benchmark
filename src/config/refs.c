#include "refs.h"

// Starter references (rough). After your first stable run, copy your
// own machine's metrics here to make indices ~1.0 on that box.
static const BenchRefs REFS = {
    .pref_integer_mips = 3000.0,   // 3,000 MIPS
    .pref_float_mflops = 50000.0,  // 50,000 MFLOPS
    .pref_mem_mbps = 25000.0,  // 25,000 MB/s (?25 GB/s)
    .pref_aes_mbps = 3000.0,   // 3,000 MB/s
    .pref_comp_mbps = 700.0     // 700 MB/s (avg comp/decomp)
};

const BenchRefs* bench_refs_defaults(void) { return &REFS; }