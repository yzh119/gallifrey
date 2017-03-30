cmake_minimum_required(VERSION 3.5)
project(gallifrey)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS -O3)

include_directories( vendor/bitmap vendor/concurrentqueue)

set(SOURCE_CODE
        src/main.cpp src/geometry.h src/shader.h src/io.h src/kdtree.h src/tracing.h src/colors.h src/image.h src/material.h src/aabb.h)

add_executable(gallifrey
        ${SOURCE_CODE} )

target_link_libraries(gallifrey
        pthread)