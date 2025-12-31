
#include <stdio.h>
#include <float.h>
#include "suite.h"
#include "config.h"
#include "refs.h"

#include "integer_mix.h"
#include "float_dot.h"
#include "memory_triad.h"
#include "aes_throughput.h"
#include "compress_throughput.h"
#include "disk_sys.h"

#include "report_csv.h"

typedef enum { PREF_INT, PREF_FP, PREF_MEM, PREF_AES, PREF_COMP, PREF_LATENCY, PREF_DISK } PrefKind;

typedef struct {
    const char* id;
    const char* title;
    const char* unit;            // "MIPS", "MFLOPS", "MB/s"
    PrefKind    prefk;
    double    (*once)(void);     // one timed pass returns throughput
} TestEntry;

static double pick_pref(PrefKind k, const BenchRefs* r) {
    switch (k) {
    case PREF_INT:  return r->pref_integer_mips;
    case PREF_FP:   return r->pref_float_mflops;
    case PREF_MEM:  return r->pref_mem_mbps;
    case PREF_AES:  return r->pref_aes_mbps;
    case PREF_COMP: return r->pref_comp_mbps;
	case PREF_LATENCY: return r ->pref_latency_mops;
	case PREF_DISK: return r->pref_disk_mbps;
    default:        return 1.0;
    }
}

void suite_run_with_callback(StatusCallback cb) {
    const int K = 5;

    // Warm-up
    (void)integer_mips_once();

    
    for (int r = 0; r < K; ++r) {
        double mips = integer_mips_once();

        // SEND DATA TO PYTHON
        // If the callback 'cb' is not NULL, call it!
        if (cb != NULL) {
            // Passing: progress (r+1), and the score (mips)
            cb(r + 1, mips);
        }
    }
}

void run_test_by_id(int id, StatusCallback cb) {
    const BenchConfig* cfg = bench_config_defaults();
    const int K = cfg->repetitionsK;

    for (int r = 0; r < K; ++r) {
        double score = 0.0;

        switch (id) {
        case TEST_INTEGER:
            if (r == 0) integer_mips_once(); // Warmup
            score = integer_mips_once();
            break;

        case TEST_FLOAT:
            if (r == 0) float_mflops_once();
            score = float_mflops_once();
            break;

        case TEST_MEMORY:
            if (r == 0) memory_mbps_once();
            score = memory_mbps_once();
            break;

        case TEST_AES:
            if (r == 0) aes_mbps_once();
            score = aes_mbps_once();
            break;

        case TEST_COMP:
            if (r == 0) compress_mbps_once();
            score = compress_mbps_once();
            break;

        case TEST_MEMORY_LATENCY:
            score = memory_random_mops_once();
            break;

        case TEST_DISK:
            if (r == 0) disk_benchmark_mbps_once();
            score = disk_benchmark_mbps_once();
			break;

        default:
            score = 0.0;
            break;
        }

        if (cb) cb(r + 1, score);
    }
}

// Helper to get reference values (for grading)
API double get_test_reference(int id) {
    const BenchRefs* refs = bench_refs_defaults();
    switch (id) {
    case TEST_INTEGER: return refs->pref_integer_mips;
    case TEST_FLOAT:   return refs->pref_float_mflops;
    case TEST_MEMORY:  return refs->pref_mem_mbps;
    case TEST_AES:     return refs->pref_aes_mbps;
    case TEST_COMP:    return refs->pref_comp_mbps;
    case TEST_MEMORY_LATENCY: return refs->pref_latency_mops;
    case TEST_DISK:    return refs->pref_disk_mbps;
    default: return 1.0;
    }
}


void suite_run_all(void) {
    const BenchConfig* cfg = bench_config_defaults();
    const BenchRefs* ref = bench_refs_defaults();

    // Single source of truth for the registry:
    static const TestEntry tests[] = {
        { "INT", "Integer checksum mix",     "MIPS",   PREF_INT,  integer_mips_once },
        { "FP",  "Floating-point dot",       "MFLOPS", PREF_FP,   float_mflops_once },
        { "MEM", "Memory TRIAD (A=B+s*C)",   "MB/s",   PREF_MEM,  memory_mbps_once },
        { "AES", "AES-128 ECB (throughput)", "MB/s",   PREF_AES,  aes_mbps_once },
        { "CMP", "DEFLATE-style codec",      "MB/s",   PREF_COMP, compress_mbps_once },
        { "RND", "Memory Random Latency",    "MOPS",   PREF_LATENCY, memory_random_mops_once },
		{ "DSK", "Disk I/O Throughput",     "MB/s",   PREF_DISK, disk_benchmark_mbps_once }
    };
    const int T = (int)(sizeof(tests) / sizeof(tests[0]));

    printf("=== PC Benchmark (multi-algorithm run) ===\n");
    printf("K=%d, warmup=%d\n\n", cfg->repetitionsK, cfg->warmup);

    double grade_sum = 0.0;
    
    //Open
    FILE* csv = report_csv_begin("results/run.csv");

    for (int t = 0; t < T; ++t) {
        const TestEntry* e = &tests[t];

        // warm-up (unmeasured)
        for (int w = 0; w < cfg->warmup; ++w) (void)e->once();

        double sum = 0.0, minv = DBL_MAX, maxv = 0.0;
        for (int r = 0; r < cfg->repetitionsK; ++r) {
            double v = e->once();
            if (v < minv) minv = v;
            if (v > maxv) maxv = v;
            sum += v;
            printf("[%s] run %d/%d: %.1f %s\n", e->id, r + 1, cfg->repetitionsK, v, e->unit);

            
        }

        const double avg = sum / (double)cfg->repetitionsK;
        const double pref = pick_pref(e->prefk, ref);
        const double index = (pref > 0.0) ? (avg / pref) : 0.0;

        if (csv) {
            report_csv_write(csv, e->id, e->title, e->unit, avg, minv, maxv, index);
        }

        printf("  -> %s average: %.1f %s  [min %.1f, max %.1f], index = %.3f\n\n",
            e->title, avg, e->unit, minv, maxv, index);

        grade_sum += index;
    }

    report_csv_end(csv);

    const double final_grade = grade_sum / (double)T;
    printf("=== Final grade (mean of indices over %d algorithms): %.3f ===\n", T, final_grade);
}
