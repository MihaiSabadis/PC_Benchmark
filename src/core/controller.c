#include <stdio.h>
#include "controller.h"
#include "suite.h"

//includes for beta
#include "config.h"
#include "refs.h"


void run_suite(void) {
    printf("[Controller] Running full suite...\n");
    suite_run_all();
}

void run_single(const char* test_id) {
    printf("[Controller] Running single test: %s\n", test_id ? test_id : "(null)");
}