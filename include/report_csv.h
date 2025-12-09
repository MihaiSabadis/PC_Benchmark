#pragma once
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

    FILE* report_csv_begin(const char* path);  // opens/creates file and writes header if empty
    void   report_csv_write(FILE* f,
        const char* id,
        const char* title,
        const char* unit,
        double avg, double minv, double maxv, double index);
    void   report_csv_end(FILE* f);

#ifdef __cplusplus
}
#endif
