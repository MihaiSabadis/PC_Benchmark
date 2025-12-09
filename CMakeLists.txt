cmake_minimum_required(VERSION 3.20)
project(pc_bench VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# This puts the .dll and .exe in the same folder (out/build/.../bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(PUBLIC_INC ${CMAKE_SOURCE_DIR}/include)
include_directories(${PUBLIC_INC})

# --- SOURCES ---
file(GLOB SRC_APP      ${CMAKE_SOURCE_DIR}/src/app/*.c)
file(GLOB SRC_CORE     ${CMAKE_SOURCE_DIR}/src/core/*.c)
file(GLOB SRC_MODULES  ${CMAKE_SOURCE_DIR}/src/modules/*.c)
file(GLOB SRC_PIPELINE ${CMAKE_SOURCE_DIR}/src/pipeline/*.c)
file(GLOB SRC_REPORT   ${CMAKE_SOURCE_DIR}/src/report/*.c)
file(GLOB SRC_CONFIG   ${CMAKE_SOURCE_DIR}/src/config/*.c)
file(GLOB SRC_UTILS    ${CMAKE_SOURCE_DIR}/src/utils/*.c)

# --- 1. BUILD THE SHARED LIBRARY (For Python) ---
# We use 'pcbench' as the name. This creates pcbench.dll (Windows) or libpcbench.so (Linux)
add_library(pcbench SHARED
    ${SRC_CORE}
    ${SRC_MODULES}
    ${SRC_PIPELINE}
    ${SRC_REPORT}
    ${SRC_CONFIG}
    ${SRC_UTILS}
    "include/config.h" "src/config/config_defaults.c" 
    "include/refs.h" "src/config/refs.c" 
    "include/integer_mix.h"
)

# Allow other targets to find headers easily
target_include_directories(pcbench PUBLIC ${PUBLIC_INC})

# --- 2. BUILD THE CLI APP (Optional, for testing without Python) ---
# We name it 'pc-bench-cli' so it doesn't conflict with the library name
add_executable(pc-bench-cli ${SRC_APP})
target_link_libraries(pc-bench-cli PRIVATE pcbench)

# --- 3. (DISABLED) OLD C++ GUI ---
# Since you are using Python, we do not need ImGui. 
# If you ever want to go back to C++, uncomment the lines below.
# if (BUILD_GUI AND WIN32)
#   ... (ImGui logic omitted) ...
# endif()