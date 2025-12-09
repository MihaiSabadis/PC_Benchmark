#include <stdlib.h>
#include "timer.h"
#include "config.h"

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
