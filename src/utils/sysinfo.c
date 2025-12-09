#include <stdio.h>
#include <string.h>
#include "sysinfo.h"

#ifdef _WIN32
#include <windows.h>
#include <intrin.h>

API void get_system_info_str(char* buffer, int max_len) {
    // 1. Get CPU Name using CPUID
    int cpuInfo[4] = { -1 };
    char cpu_brand[0x40];
    memset(cpu_brand, 0, sizeof(cpu_brand));

    __cpuid(cpuInfo, 0x80000000);
    unsigned int nExIds = cpuInfo[0];

    // Interpret CPU brand string
    if (nExIds >= 0x80000004) {
        __cpuid(cpuInfo, 0x80000002);
        memcpy(cpu_brand, cpuInfo, sizeof(cpuInfo));
        __cpuid(cpuInfo, 0x80000003);
        memcpy(cpu_brand + 16, cpuInfo, sizeof(cpuInfo));
        __cpuid(cpuInfo, 0x80000004);
        memcpy(cpu_brand + 32, cpuInfo, sizeof(cpuInfo));
    }
    else {
        strcpy_s(cpu_brand, sizeof(cpu_brand), "Unknown CPU");
    }

    // 2. Get RAM Size
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    double ram_gb = (double)statex.ullTotalPhys / (1024 * 1024 * 1024);

    // 3. Format Output
    snprintf(buffer, max_len, "CPU: %s\nRAM: %.1f GB", cpu_brand, ram_gb);
}

#else
// Simple Linux Fallback
API void get_system_info_str(char* buffer, int max_len) {
    snprintf(buffer, max_len, "System Info: (Linux implementation pending)");
}
#endif