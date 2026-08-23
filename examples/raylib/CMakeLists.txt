cmake_minimum_required(VERSION 3.21)

include(FetchContent)

FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/6.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
)
FetchContent_MakeAvailable(raylib)

add_executable(sa_example_raylib main.c)
target_link_libraries(sa_example_raylib PRIVATE
    raylib
    suzuki_abe_border_topology::suzuki_abe_border_topology)


set_target_properties(sa_example_raylib PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
    C_EXTENSION NO
)

target_compile_options(sa_example_raylib PRIVATE -Wall -Wextra -pedantic -Werror)

add_custom_command(TARGET sa_example_raylib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/suzuki_abe_test.png"
    "$<TARGET_FILE_DIR:sa_example_raylib>/suzuki_abe_test.png"
    VERBATIM
)

add_custom_command(TARGET sa_example_raylib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/suzuki_abe_test2.png"
    "$<TARGET_FILE_DIR:sa_example_raylib>/suzuki_abe_test2.png"
    VERBATIM
)
