#include "disk_sys.h"
#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
#include "config.h"

double disk_benchmark_mbps_once(void) {
    const BenchConfig* cfg = bench_config_defaults();
    const size_t FILE_SIZE = cfg->disk_bytes;
    const size_t CHUNK_SIZE = 4096; // 4KB blocks
    const char* filename = "scs_bench_temp.dat";

    char* buffer = (char*)malloc(CHUNK_SIZE);
    if (!buffer) return 0.0;

    // Write
    FILE* f = fopen(filename, "wb");
    if (!f) { free(buffer); return 0.0; }

    timer_start();
    for (size_t i = 0; i < FILE_SIZE; i += CHUNK_SIZE) {
        fwrite(buffer, 1, CHUNK_SIZE, f);
    }
    double t_write = timer_elapsed_seconds();
    fclose(f);

    // Read
    f = fopen(filename, "rb");
    if (!f) { free(buffer); return 0.0; }

    timer_start();
    while (fread(buffer, 1, CHUNK_SIZE, f) == CHUNK_SIZE) {
        // consume data
    }
    double t_read = timer_elapsed_seconds();
    fclose(f);

    // Cleanup
    remove(filename);
    free(buffer);

    // Calculate Combined Throughput (Total Bytes / Total Time)
    // We processed FILE_SIZE twice (Write + Read)
    double total_bytes = (double)FILE_SIZE * 2.0;
    double total_time = t_write + t_read;

    if (total_time <= 0.0) return 0.0;

    return (total_bytes / total_time) / (1024.0 * 1024.0); // MB/s
}