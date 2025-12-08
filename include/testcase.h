#pragma once
typedef struct {
    const char* id;           // "integer", "float", "memory", "aes", "compress"
    const char* title;        // human-readable
    void (*prepare)(void);    // allocate/fill data, optional warm-up
    void (*run_once)(void);   // one repetition (unit of work)
    const char* unit;         // "MIPS", "MFLOPS", "MB/s"
} TestCase;
