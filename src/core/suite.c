#include <stdio.h>
#include <float.h>
#include "suite.h"
#include "integer_mix.h"

void suite_run_all(void) {
    const int K = 5;  // repetitions (basic default)
    // optional warm-up (unmeasured)
    (void)integer_mips_once();

    double sum = 0.0, minv = DBL_MAX, maxv = 0.0;
    for (int r = 0; r < K; ++r) {
        double mips = integer_mips_once();
        if (mips < minv) minv = mips;
        if (mips > maxv) maxv = mips;
        sum += mips;
        printf("[run %d/%d] Integer: %.1f MIPS\n", r + 1, K, mips);
    }
    double avg = sum / K;
    printf("----------------------------------------\n");
    printf("Integer average (K=%d): %.1f MIPS  [min %.1f, max %.1f]\n",
        K, avg, minv, maxv);
}
