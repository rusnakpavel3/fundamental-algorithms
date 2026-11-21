cmake_minimum_required(VERSION 3.10)
project(lab3_task9 C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

if (MINGW)
    add_compile_definitions(__USE_MINGW_ANSI_STDIO=1)
endif ()

add_executable(lab3_task9 main.c status.c word_tree.c text_reader.c tree_file.c same_file.c console.c)
