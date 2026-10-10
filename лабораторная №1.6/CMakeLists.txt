cmake_minimum_required(VERSION 3.10)
project(lab1_task6 C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

if (MINGW)
    add_compile_definitions(__USE_MINGW_ANSI_STDIO=1)
endif ()

add_executable(lab1_task6 main.c variadic.c power.c bisection.c status.c)
target_link_libraries(lab1_task6 m)
