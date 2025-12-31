#include <stdlib.h>
#include "timer.h"
#include "config.h"
#include <stdint.h>
#include "memory_triad.h"

// for fast deterministic pseudo-random numbers
static inline uint32_t fast_rand(uint32_t* seed) {
    *seed = *seed * 1103515245 + 12345;
    return (*seed / 65536) % 32768;
}

double memory_random_mops_once(void) {
    const BenchConfig* cfg = bench_config_defaults();
    const size_t N = cfg->triad_N; // Use same array size

    // We allocate a large array of "indices" to jump around
    uint32_t* indices = (uint32_t*)malloc(N * sizeof(uint32_t));
    float* data = (float*)malloc(N * sizeof(float));

    if (!indices || !data) { free(indices); free(data); return 0.0; }

    // Pre-compute random jump pattern (Pointer Chasing setup)
    // We create a cycle so we can just do index = indices[index]
    // For simplicity here, we just fill random indices
    uint32_t seed = 42;
    for (size_t i = 0; i < N; ++i) {
        indices[i] = fast_rand(&seed) % (uint32_t)N;
        data[i] = (float)i;
    }

    // Timed Random Access
    timer_start();
    volatile float sum = 0.0f;
    size_t idx = 0;

    // We do N accesses
    for (size_t i = 0; i < N; ++i) {
        // Jump to a random location determined by pre-calc table
        // This stresses latency because CPU cannot predict 'indices[i]' easily
        idx = indices[i];
        sum += data[idx];
    }
    double dt = timer_elapsed_seconds();

    free(indices); free(data);

    // Result in Millions of Operations Per Second (MOPS)
    // This is a proxy for Latency. (1/MOPS = avg latency in microseconds)
    return ((double)N / dt) / 1e6;
}

double memory_mbps_once(void) {
    const BenchConfig* cfg = bench_config_defaults();
    const size_t N = cfg->triad_N;

    float* A = (float*)malloc(N * sizeof(float));
    float* B = (float*)malloc(N * sizeof(float));
    float* C = (float*)malloc(N * sizeof(float));
    if (!A || !B || !C) { free(A); free(B); free(C); return 0.0; }

    // Init (outside timing)
    for (size_t i = 0; i < N; ++i) {
        B[i] = (float)((i % 251) * 0.01f);
        C[i] = (float)(((i * 5) % 233) * 0.01f);
    }
    const float s = 2.0f;

    // Time one streaming pass: A = B + s*C
    timer_start();
    for (size_t i = 0; i < N; ++i) {
        A[i] = B[i] + s * C[i];
    }
    double dt = timer_elapsed_seconds();

    // Prevent optimization
    volatile float sink = A[0]; (void)sink;

    free(A); free(B); free(C);

    // Bytes moved: read B + read C + write A (3 arrays)
    const double bytes = (double)N * 3.0 * sizeof(float);
    return (bytes / dt) / (1024.0 * 1024.0);  // MB/s (MiB/s)
}
