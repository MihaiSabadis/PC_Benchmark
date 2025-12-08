#include <stdio.h>
#include <stdint.h>
#include "timer.h"
#include "util.h"
#include "metric_units.h"

void integer_demo_run(void) {
    const size_t N = 50 * 1000 * 1000ULL; // 50M byte-like steps
    uint64_t acc = 0x9e3779b97f4a7c15ULL;
    timer_start();
    for (size_t i = 0; i < N; i++) {
        acc ^= (uint64_t)i;
        acc *= 0x2545F4914F6CDD1DULL;
        acc = rotl64(acc, 13);
    }
    const double dt = timer_elapsed_seconds();
    const double ops = (double)N * 3.0; // xor + mul + rot
    const double mips = (ops / dt) / 1e6;
    printf("[Integer] ~%.1f %s (acc=%llu)\\n", mips, UNIT_MIPS, (unsigned long long)acc);
}


double integer_mips_once(void) {
    const uint64_t C1 = 0x2545F4914F6CDD1DULL;
    const size_t   N = 50ull * 1000ull * 1000ull; // 50M iterations

    uint64_t acc = 0x9e3779b97f4a7c15ULL;

    // time the loop
    timer_start();
    for (size_t i = 0; i < N; ++i) {
        acc ^= (uint64_t)i;     // xor
        acc *= C1;              // multiply
        acc = rotl64(acc, 13);  // rotate
    }
    double dt = timer_elapsed_seconds();

    // prevent the compiler from discarding the work
    volatile uint64_t sink = acc; (void)sink;

    const double ops = (double)N * 3.0;   // xor + mul + rot = 3 ops
    const double mips = (ops / dt) / 1e6;  // millions of ops per second
    return mips;
}