#include "refs.h"

// Calibrated on: <your PC>, Win11, VS2022 x64-Release
// Sizes: INT 64MiB x4, FLOAT N=4,194,304, TRIAD N=16,777,216, AES 128MiB, COMP 64MiB
// Date: <2025-12-09>. Method: median of 5 run averages.

static const BenchRefs REFS = {
    .pref_integer_mips = 2265.930017,  // INT  (MIPS)
    .pref_float_mflops = 888.615464,  // FP   (MFLOPS)
    .pref_mem_mbps = 11190.759438,  // MEM  (MB/s)
    .pref_aes_mbps = 1581.968614,  // AES  (MB/s)
    .pref_comp_mbps = 710.712300   // CMP  (MB/s)
};


const BenchRefs* bench_refs_defaults(void) { return &REFS; }

//random and sequencial for memory access and test