#include <stdlib.h>
#include <stdint.h>
#include "timer.h"
#include "config.h"
#include "util.h"

double aes_mbps_once(void) {
    const size_t V = bench_config_defaults()->aes_bytes; // total bytes
    uint8_t* buf = (uint8_t*)malloc(V);
    if (!buf) return 0.0;

    // init not timed
    for (size_t i = 0; i < V; i++) buf[i] = (uint8_t)(i * 131u);

    // timing: simulate 10 "rounds" over 16B blocks (ECB-like, no I/O)
    timer_start();
    for (size_t off = 0; off + 16 <= V; off += 16) {
        uint32_t* w = (uint32_t*)(buf + off);
        // 10 very cheap rounds to mimic steady compute on 16B
        for (int r = 0; r < 10; r++) {
            w[0] ^= 0x243F6A88u; w[0] = rotl32(w[0] + w[1], 5);
            w[1] ^= 0x85A308D3u; w[1] = rotl32(w[1] + w[2], 7);
            w[2] ^= 0x13198A2Eu; w[2] = rotl32(w[2] + w[3], 11);
            w[3] ^= 0x03707344u; w[3] = rotl32(w[3] + w[0], 13);
        }
    }
    double dt = timer_elapsed_seconds();

    volatile uint8_t sink = buf[0]; (void)sink;
    free(buf);

    return ((double)V / dt) / (1024.0 * 1024.0); // MB/s
}
