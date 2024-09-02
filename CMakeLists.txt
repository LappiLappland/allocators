cmake_minimum_required(VERSION 3.5.0)
project(allocators VERSION 0.1.0 LANGUAGES C)

add_executable(allocators main.c)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

ADD_LIBRARY(LibsModule 
    stackAllocator.c
    poolAllocator.c
    freeListAllocator.c
    commonAllocator.h
)

ADD_EXECUTABLE(laplas main.c)
target_link_libraries(allocators LibsModule)