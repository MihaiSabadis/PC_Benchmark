#include "timer.h"
#ifdef _WIN32
#include <windows.h>
static LARGE_INTEGER freq, t0;
void timer_start(void) {
    QueryPerformanceFrequency(&freq); QueryPerformanceCounter(&t0);
}

double timer_elapsed_seconds(void) {
    LARGE_INTEGER t1;
    QueryPerformanceCounter(&t1);
    return (double)(t1.QuadPart - t0.QuadPart) / (double)freq.QuadPart;
}
#else
#include <time.h>
static struct timespec t0;
void timer_start(void) { clock_gettime(CLOCK_MONOTONIC, &t0); }
double timer_elapsed_seconds(void) {
    struct timespec t1;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    return (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
}
#endif
