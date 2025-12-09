#include <stdlib.h>
#include <stdint.h>
#include "timer.h"
#include "config.h"

double float_mflops_once(void) {
    const BenchConfig* cfg = bench_config_defaults();
    const size_t N = cfg->float_N;

    float* a = (float*)malloc(N * sizeof(float));
    float* b = (float*)malloc(N * sizeof(float));
    if (!a || !b) { free(a); free(b); return 0.0; }

    // Deterministic init (kept outside timing)
    for (size_t i = 0; i < N; ++i) {
        a[i] = (float)((i % 97) * 0.01);
        b[i] = (float)(((i * 3) % 89) * 0.01);
    }

    // Time only the math loop
    volatile double sum = 0.0;
    timer_start();
    for (size_t i = 0; i < N; ++i) {
        sum += (double)a[i] * (double)b[i];  // 2 FLOPs per element (mul + add)
    }
    double dt = timer_elapsed_seconds();

    // Prevent optimization
    (void)sum;

    free(a); free(b);

    const double flops = 2.0 * (double)N;
    return (flops / dt) / 1e6;   // MFLOPS
}
