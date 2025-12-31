#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sysinfo.h"


// WINDOWS IMPLEMENTATION
#ifdef _WIN32
#include <windows.h>
#include <intrin.h>

static void get_gpu_name(char* buffer, int max_len) {
    DISPLAY_DEVICEA dd;
    dd.cb = sizeof(dd);
    if (EnumDisplayDevicesA(NULL, 0, &dd, 0)) {
        strncpy_s(buffer, max_len, dd.DeviceString, _TRUNCATE);
    }
    else {
        strncpy_s(buffer, max_len, "Unknown GPU", _TRUNCATE);
    }
}

static void get_os_name(char* buffer, int max_len) {
    HKEY hKey;
    char productName[256] = "Windows (Unknown)";
    DWORD bufLen = sizeof(productName);
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExA(hKey, "ProductName", NULL, NULL, (LPBYTE)productName, &bufLen);
        RegCloseKey(hKey);
    }
    strncpy_s(buffer, max_len, productName, _TRUNCATE);
}

API void get_system_info_str(char* buffer, int max_len) {
    // CPU Brand
    int cpuInfo[4] = { -1 };
    char cpu_brand[0x40];
    memset(cpu_brand, 0, sizeof(cpu_brand));
    __cpuid(cpuInfo, 0x80000000);
    if (cpuInfo[0] >= 0x80000004) {
        __cpuid(cpuInfo, 0x80000002); memcpy(cpu_brand, cpuInfo, 16);
        __cpuid(cpuInfo, 0x80000003); memcpy(cpu_brand + 16, cpuInfo, 16);
        __cpuid(cpuInfo, 0x80000004); memcpy(cpu_brand + 32, cpuInfo, 16);
    }
    else strcpy_s(cpu_brand, sizeof(cpu_brand), "Unknown CPU");

    // RAM
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    double ram_gb = (double)statex.ullTotalPhys / (1024 * 1024 * 1024);

    // Cores
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    int num_cores = sysInfo.dwNumberOfProcessors;

    // GPU & OS
    char gpu_name[128]; get_gpu_name(gpu_name, sizeof(gpu_name));
    char os_name[128];  get_os_name(os_name, sizeof(os_name));

    snprintf(buffer, max_len,
        "OS:  %s\nCPU: %s\n     (%d Logical Cores)\nGPU: %s\nRAM: %.1f GB",
        os_name, cpu_brand, num_cores, gpu_name, ram_gb);
}

// LINUX & MACOS IMPLEMENTATION
#else
#include <unistd.h>
#include <sys/utsname.h>

// Helper to run a shell command and get the first line of output
static void get_cmd_output(const char* cmd, char* buffer, int max_len) {
    FILE* fp = popen(cmd, "r");
    if (fp) {
        if (fgets(buffer, max_len, fp) != NULL) {
            // Remove newline at end
            buffer[strcspn(buffer, "\n")] = 0;
        }
        else {
            strncpy(buffer, "Unknown", max_len);
        }
        pclose(fp);
    }
    else {
        strncpy(buffer, "Error", max_len);
    }
}

API void get_system_info_str(char* buffer, int max_len) {
    // OS Name (Kernel version)
    struct utsname os_info;
    char os_str[128] = "Linux/Unix";
    if (uname(&os_info) == 0) {
        snprintf(os_str, sizeof(os_str), "%s %s", os_info.sysname, os_info.release);
    }

    // CPU Model & Cores
    char cpu_model[128] = "Unknown CPU";
    long num_cores = sysconf(_SC_NPROCESSORS_ONLN);

    // Linux-specific way to get CPU name
#ifdef __linux__
    get_cmd_output("grep -m1 'model name' /proc/cpuinfo | cut -d: -f2 | sed 's/^ *//'", cpu_model, sizeof(cpu_model));
#elif __APPLE__
    get_cmd_output("sysctl -n machdep.cpu.brand_string", cpu_model, sizeof(cpu_model));
#endif

    // RAM Size
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    double ram_gb = (double)(pages * page_size) / (1024.0 * 1024.0 * 1024.0);

    // GPU (Tricky on Linux, requires tools like lspci)
    char gpu_name[128] = "Unknown (requires lspci)";
#ifdef __linux__
    // Try to get the first VGA compatible controller
    get_cmd_output("lspci | grep -i 'vga\\|3d' | head -n 1 | cut -d: -f3 | sed 's/^ *//'", gpu_name, sizeof(gpu_name));
#elif __APPLE__
    get_cmd_output("system_profiler SPDisplaysDataType | grep 'Chipset Model' | head -n 1 | cut -d: -f2 | sed 's/^ *//'", gpu_name, sizeof(gpu_name));
#endif

    snprintf(buffer, max_len,
        "OS:  %s\nCPU: %s\n     (%ld Logical Cores)\nGPU: %s\nRAM: %.1f GB",
        os_str, cpu_model, num_cores, gpu_name, ram_gb);
}
#endif