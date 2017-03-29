cmake_minimum_required(VERSION 3.5)
project(gallifrey)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS -O3)

set(BITMAP_PATH
        vendor/bitmap)

include_directories( ${BITMAP_PATH} )

set(SOURCE_CODE
        src/main.cpp src/geometry.h src/shader.h src/io.h src/kdtree.h src/tracing.h src/colors.h src/image.h src/material.h)

add_executable(gallifrey
        ${SOURCE_CODE} )