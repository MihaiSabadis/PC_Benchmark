#pragma once
#include <stddef.h>

// Define exports for Windows DLL
#ifdef _WIN32
#ifdef pcbench_EXPORTS
#define API __declspec(dllexport)
#else
#define API __declspec(dllimport)
#endif
#else
#define API
#endif

typedef struct {
    int    repetitionsK;          // runs per module
    int    warmup;                // unmeasured warmup runs
    size_t integer_block_bytes;   // B
    int    integer_passes;        // R
    size_t float_N;               // elements for dot product
    size_t triad_N;               // elements for Triad
    size_t aes_bytes;             // V_data for AES
    size_t comp_bytes;            // V_data for Compression
	size_t disk_bytes;            // total bytes for Disk I/O
} BenchConfig;

#ifdef __cplusplus
extern "C" {
#endif
    // Returns the current configuration (mutable)
    API BenchConfig* bench_config_defaults(void);

    // 0=Quick, 1=Standard, 2=Extreme
    API void set_config_profile(int profile_id);
#ifdef __cplusplus
}
#endif