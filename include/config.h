#pragma once
#include <stddef.h>

typedef struct {
    int    repetitionsK;          // runs per module
    int    warmup;                // unmeasured warmup runs
    size_t integer_block_bytes;   // B
    int    integer_passes;        // R
    size_t float_N;               // elements for dot product
    size_t triad_N;               // elements for Triad
    size_t aes_bytes;             // V_data for AES
    size_t comp_bytes;            // V_data for Compression
} BenchConfig;

#ifdef __cplusplus
extern "C" {
#endif
    const BenchConfig* bench_config_defaults(void);
#ifdef __cplusplus
}
#endif
