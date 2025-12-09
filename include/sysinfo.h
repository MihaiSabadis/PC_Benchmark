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

API void get_system_info_str(char* buffer, int max_len);