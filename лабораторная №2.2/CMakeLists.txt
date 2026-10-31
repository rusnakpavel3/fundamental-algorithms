cmake_minimum_required(VERSION 3.10)
project(lab2_task2 C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

if (MINGW)
    add_compile_definitions(__USE_MINGW_ANSI_STDIO=1)
endif ()

add_executable(lab2_task2 main.c status.c employee.c file_io.c same_file.c)
target_link_libraries(lab2_task2 m)
