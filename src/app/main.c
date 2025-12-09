#include <stdio.h>
#include "controller.h"
#include "sysinfo.h"  // <--- You must include this to see get_system_info_str

int main(void) {
    // 1. Get the system info string
    char info_buffer[512];
    get_system_info_str(info_buffer, sizeof(info_buffer));

    // 2. Print it
    printf("=== System Info ===\n%s\n\n", info_buffer);

    // 3. Run the benchmark suite
    run_suite();
    return 0;
}