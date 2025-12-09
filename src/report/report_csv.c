#include "report_csv.h"
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define MKDIR(p) _mkdir(p)   // returns -1 if exists; fine to ignore
#else
#include <sys/stat.h>
#define MKDIR(p) mkdir(p, 0755)
#endif

static void ensure_results_dir(void) {
    (void)MKDIR("results");
}

FILE* report_csv_begin(const char* path) {
    ensure_results_dir();
    FILE* f = fopen(path, "a+");        // append (create if missing)
    if (!f) return NULL;

    // Write header if file is empty
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    if (sz == 0) {
        fprintf(f, "id,title,unit,avg,min,max,index\n");
        fflush(f);
    }
    return f;
}

// Replace commas in the title so CSV stays valid
static void csv_sanitize_title(const char* in, char* out, size_t n) {
    size_t i = 0;
    while (in && *in && i < n - 1) {
        out[i++] = (*in == ',') ? ' ' : *in;
        ++in;
    }
    out[i] = '\0';
}

void report_csv_write(FILE* f, const char* id, const char* title, const char* unit,
    double avg, double minv, double maxv, double index) {
    if (!f) return;
    char safe[256];
    csv_sanitize_title(title, safe, sizeof(safe));
    fprintf(f, "%s,%s,%s,%.6f,%.6f,%.6f,%.6f\n",
        id, safe, unit, avg, minv, maxv, index);
    fflush(f);
}

void report_csv_end(FILE* f) {
    if (f) fclose(f);
}
