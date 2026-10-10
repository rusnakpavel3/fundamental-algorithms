cmake_minimum_required(VERSION 3.10)
project(lab1_task7 C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

if (MINGW)
    add_compile_definitions(__USE_MINGW_ANSI_STDIO=1)
endif ()

add_executable(lab1_task7 main.c tokens.c transform.c paths.c same_file.c status.c)
