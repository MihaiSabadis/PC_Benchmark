#include <stdio.h>
#include "controller.h"
#include "sysinfo.h"
#include "config.h"

int main(void) {
    // Get and print system info
    char info_buffer[512];
    get_system_info_str(info_buffer, sizeof(info_buffer));
    printf("=== System Info ===\n%s\n\n", info_buffer);

    
    int profile = -1;
    printf("Choose profile to run:\n");
    printf("  0 - QUICK\n");
    printf("  1 - STANDARD\n");
    printf("  2 - EXTREME\n");
    printf("Enter profile id (0/1/2): ");
    if (scanf("%d", &profile) != 1) {
        fprintf(stderr, "Invalid input.\n");
        return 1;
    }
    if (profile < 0 || profile > 2) {
        fprintf(stderr, "Profile id must be 0, 1, or 2.\n");
        return 1;
    }

    
    const char* names[] = { "QUICK (0)", "STANDARD (1)", "EXTREME (2)" };
    printf("\n>>> RUNNING PROFILE: %s <<<\n", names[profile]);
    set_config_profile(profile);
    run_suite();

    return 0;
}