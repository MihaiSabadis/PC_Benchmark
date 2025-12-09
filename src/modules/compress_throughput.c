// src/modules/compress_throughput.c
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "compress_throughput.h"
#include "timer.h"
#include "config.h"

double compress_mbps_once(void) {
    const BenchConfig* cfg = bench_config_defaults();
    const size_t V = cfg->comp_bytes;          // total input bytes

    uint8_t* in = (uint8_t*)malloc(V);
    const size_t TMP_CAP = V ? (2 * V) : 2;    // worst case: 2 bytes per input byte
    uint8_t* tmp = (uint8_t*)malloc(TMP_CAP);
    uint8_t* out = (uint8_t*)malloc(V);        // decompressed size <= V for this toy RLE
    if (!in || !tmp || !out) { free(in); free(tmp); free(out); return 0.0; }

    // init (not timed)
    for (size_t i = 0; i < V; ++i) in[i] = (uint8_t)((i * 7u) ^ (i >> 3));

    timer_start();

    // "compress": scan & encode short runs (toy placeholder)
    size_t w = 0;
    for (size_t i = 0; i < V; ) {
        uint8_t b = in[i];
        size_t run = 1;
        while (i + run < V && in[i + run] == b && run < 255) run++;
        if (w + 2 > TMP_CAP) break;        // safety guard (shouldn’t trigger with 2*V)
        tmp[w++] = b;
        tmp[w++] = (uint8_t)run;
        i += run;
    }

    // "decompress": expand back
    size_t r = 0;
    for (size_t i = 0; i + 1 < w; i += 2) {
        uint8_t b = tmp[i];
        size_t cnt = (size_t)tmp[i + 1];
        if (r + cnt > V) cnt = V - r;      // clamp to out buffer
        memset(out + r, b, cnt);
        r += cnt;
        if (r >= V) break;
    }

    double dt = timer_elapsed_seconds();

    volatile uint8_t sink = out[r ? (r - 1) : 0]; (void)sink; // keep side effects
    free(in); free(tmp); free(out);

    const double bytes = (double)V + (double)r;   // compress + decompress
    return (bytes / dt) / (1024.0 * 1024.0);      // MB/s
}
