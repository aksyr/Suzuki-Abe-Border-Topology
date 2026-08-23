cmake_minimum_required(VERSION 3.21)
project(suzuki_abe_border_topology)

add_library(suzuki_abe_border_topology STATIC src/sa_border_topology.c)
add_library(suzuki_abe_border_topology::suzuki_abe_border_topology ALIAS suzuki_abe_border_topology)

target_include_directories(suzuki_abe_border_topology PUBLIC include)

set_target_properties(suzuki_abe_border_topology PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
    C_EXTENSION NO
)

target_compile_options(suzuki_abe_border_topology PRIVATE -Wall -Wextra -pedantic -Werror)

option(SA_BUILD_EXAMPLES "Build examples" ${PROJECT_IS_TOP_LEVEL})

if(SA_BUILD_EXAMPLES)
    add_subdirectory(examples/raylib)
endif()