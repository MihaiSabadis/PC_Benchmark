#include <stdio.h>
#include "controller.h"
#include "sysinfo.h"
#include "config.h"

int main(void) {
    // 1. Get and print system info
    char info_buffer[512];
    get_system_info_str(info_buffer, sizeof(info_buffer));
    printf("=== System Info ===\n%s\n\n", info_buffer);

    // 2. Run Profile 0: Quick
    printf("\n>>> RUNNING PROFILE: QUICK (0) <<<\n");
    set_config_profile(0);
    run_suite();

    // 3. Run Profile 1: Standard
    printf("\n>>> RUNNING PROFILE: STANDARD (1) <<<\n");
    set_config_profile(1);
    run_suite();

    // 4. Run Profile 2: Extreme
    printf("\n>>> RUNNING PROFILE: EXTREME (2) <<<\n");
    set_config_profile(2);
    run_suite();

    return 0;
}