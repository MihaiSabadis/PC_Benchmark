#pragma once

typedef struct {
    double pref_integer_mips;   // ops/s ? MIPS
    double pref_float_mflops;   // FLOPs/s ? MFLOPS
    double pref_mem_mbps;       // MB/s  (Triad)
    double pref_aes_mbps;       // MB/s  (encrypt)
    double pref_comp_mbps;      // MB/s  (avg of comp+decomp)
	double pref_latency_mops;   // MOPS (Memory Latency)
} BenchRefs;

#ifdef __cplusplus
extern "C" {
#endif
    const BenchRefs* bench_refs_defaults(void);
#ifdef __cplusplus
}
#endif