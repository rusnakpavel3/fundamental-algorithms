cmake_minimum_required(VERSION 3.10)
project(lab3_task10 C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

if (MINGW)
    add_compile_definitions(__USE_MINGW_ANSI_STDIO=1)
endif ()

add_executable(lab3_task10 main.c status.c bracket_tree.c tree_print.c line_reader.c same_file.c)
