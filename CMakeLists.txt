cmake_minimum_required(VERSION 3.20)
project(bl C)

set(CMAKE_C_STANDARD 99)

add_library(bl library.c bl_port.h)

add_executable(test main.c)

target_link_libraries(test bl)
