cmake_minimum_required(VERSION 3.10)
project(lab2_task9 C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

add_executable(lab2_task9 main.c status.c fraction.c)
target_link_libraries(lab2_task9 m)
