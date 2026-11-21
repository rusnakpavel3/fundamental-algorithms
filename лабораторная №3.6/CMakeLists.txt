cmake_minimum_required(VERSION 3.10)
project(lab3_task6 C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)
if (MINGW)
    add_compile_definitions(__USE_MINGW_ANSI_STDIO=1)
endif ()

add_executable(lab3_task6 main.c status.c datetime.c record.c routes.c stop_file.c line_reader.c console.c)
target_link_libraries(lab3_task6 m)
