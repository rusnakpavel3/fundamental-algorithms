cmake_minimum_required(VERSION 3.10)
project(lab2_task8 C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

add_executable(lab2_task8 main.c status.c digits.c big_sum.c)
