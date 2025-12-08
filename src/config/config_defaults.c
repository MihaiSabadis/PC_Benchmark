#include "config.h"

static const BenchConfig CFG = {
    .repetitionsK = 5,
    .warmup = 1,
    .integer_block_bytes = 64ull * 1024ull * 1024ull,   // 64 MiB
    .integer_passes = 4,
    .float_N = 4194304ull,                  // 4 Mi elements
    .triad_N = 16777216ull,                 // 16 Mi elements
    .aes_bytes = 128ull * 1024ull * 1024ull,  // 128 MiB
    .comp_bytes = 64ull * 1024ull * 1024ull   // 64 MiB
};

const BenchConfig* bench_config_defaults(void) { return &CFG; }
