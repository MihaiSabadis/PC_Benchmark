#pragma once

// Define a macro to handle Windows DLL exports
#ifdef _WIN32
#ifdef pcbench_EXPORTS // CMake defines this automatically when building the DLL
#define API __declspec(dllexport)
#else
#define API __declspec(dllimport)
#endif
#else
#define API
#endif

// Apply the macro to the functions you need to use outside the DLL
API void run_suite(void);
API void run_single(const char* test_id);