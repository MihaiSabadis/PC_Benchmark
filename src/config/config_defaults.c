#include "config.h"

// Remove 'const' so we can modify it
static BenchConfig CFG = {
    .repetitionsK = 5,
    .warmup = 1,
    .integer_block_bytes = 64ull * 1024ull * 1024ull,   // 64 MiB
    .integer_passes = 4,
    .float_N = 4194304ull,                  // 4 Mi elements
    .triad_N = 16777216ull,                 // 16 Mi elements
    .aes_bytes = 128ull * 1024ull * 1024ull,  // 128 MiB
    .comp_bytes = 64ull * 1024ull * 1024ull,   // 64 MiB
	.disk_bytes = 128ull * 1024ull * 1024ull    // 100 MiB
};

BenchConfig* bench_config_defaults(void) { return &CFG; }

void set_config_profile(int profile_id) {
    switch (profile_id) {
    case 0: // QUICK / DEMO
        CFG.repetitionsK = 2;
        CFG.integer_block_bytes = 16ull * 1024ull * 1024ull; // 16 MB
        CFG.integer_passes = 2;
        CFG.float_N = 1048576ull;         // 1 Mi elements
        CFG.triad_N = 4194304ull;         // 4 Mi elements
        CFG.aes_bytes = 32ull * 1024ull * 1024ull;
        CFG.comp_bytes = 16ull * 1024ull * 1024ull;
		CFG.disk_bytes = 32ull * 1024ull * 1024ull;
        break;

    case 2: // EXTREME / STRESS
        CFG.repetitionsK = 10;
        CFG.integer_block_bytes = 256ull * 1024ull * 1024ull; // 256 MB
        CFG.integer_passes = 8;
        CFG.float_N = 16777216ull;        // 16 Mi elements
        CFG.triad_N = 67108864ull;        // 64 Mi elements (approx 768MB RAM used)
        CFG.aes_bytes = 512ull * 1024ull * 1024ull;
        CFG.comp_bytes = 256ull * 1024ull * 1024ull;
		CFG.disk_bytes = 512ull * 1024ull * 1024ull;
        break;

    case 1: // STANDARD (Default)
    default:
        CFG.repetitionsK = 5;
        CFG.integer_block_bytes = 64ull * 1024ull * 1024ull;
        CFG.integer_passes = 4;
        CFG.float_N = 4194304ull;
        CFG.triad_N = 16777216ull;
        CFG.aes_bytes = 128ull * 1024ull * 1024ull;
        CFG.comp_bytes = 64ull * 1024ull * 1024ull;
		CFG.disk_bytes = 128ull * 1024ull * 1024ull;
        break;
    }
}