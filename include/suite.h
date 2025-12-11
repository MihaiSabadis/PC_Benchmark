#pragma once

#ifdef _WIN32
#ifdef pcbench_EXPORTS
#define API __declspec(dllexport)
#else
#define API __declspec(dllimport)
#endif
#else
#define API
#endif

typedef void (*StatusCallback)(int progress, double score);

// IDs for the tests
#define TEST_INTEGER 0
#define TEST_FLOAT   1
#define TEST_MEMORY  2
#define TEST_AES     3
#define TEST_COMP    4
#define TEST_MEMORY_LATENCY 5

API void run_test_by_id(int id, StatusCallback cb);
API double get_test_reference(int id);