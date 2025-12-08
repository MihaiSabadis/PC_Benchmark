cmake_minimum_required(VERSION 3.20)
project(pc_bench VERSION 0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(PUBLIC_INC ${CMAKE_SOURCE_DIR}/include)
include_directories(${PUBLIC_INC})

# Collect sources
file(GLOB SRC_APP      ${CMAKE_SOURCE_DIR}/src/app/*.c)
file(GLOB SRC_CORE     ${CMAKE_SOURCE_DIR}/src/core/*.c)
file(GLOB SRC_MODULES  ${CMAKE_SOURCE_DIR}/src/modules/*.c)   # <-- add this
# (Add these later as you create them)
file(GLOB SRC_PIPELINE ${CMAKE_SOURCE_DIR}/src/pipeline/*.c)
file(GLOB SRC_REPORT   ${CMAKE_SOURCE_DIR}/src/report/*.c)
file(GLOB SRC_CONFIG   ${CMAKE_SOURCE_DIR}/src/config/*.c)
file(GLOB SRC_UTILS    ${CMAKE_SOURCE_DIR}/src/utils/*.c)

add_library(pcbench STATIC
    ${SRC_CORE}
    ${SRC_MODULES}   # <-- include modules in the lib
    ${SRC_PIPELINE}
    ${SRC_REPORT}
    ${SRC_CONFIG}
    ${SRC_UTILS}
 "include/config.h" "src/config/config_defaults.c" "include/refs.h" "src/config/refs.c" "include/integer_mix.h")
target_include_directories(pcbench PUBLIC ${PUBLIC_INC})

add_executable(pc-bench ${SRC_APP} "include/config.h" "src/config/config_defaults.c" "include/refs.h" "src/config/refs.c" "include/integer_mix.h")
target_link_libraries(pc-bench PRIVATE pcbench)
