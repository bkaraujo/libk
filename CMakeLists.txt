cmake_minimum_required(VERSION 3.30)
set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE debug)

project(libk C)

include_directories(src/main)
add_library(
        ${PROJECT_NAME} STATIC
        src/main/libk/ktime.c
        src/main/libk/klogger.c
        src/main/libk/kruntime.c
        src/main/libk/kmemory.c
        src/main/libk/kallocator.c
)
