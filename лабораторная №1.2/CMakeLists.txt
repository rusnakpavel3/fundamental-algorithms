cmake_minimum_required(VERSION 3.10)
project(lab1_task2 C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

add_executable(lab1_task2 main.c parsing.c constants.c)
target_link_libraries(lab1_task2 m)
