cmake_minimum_required(VERSION 3.5)
project(gallifrey)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -pthread")
set(BITMAP_PATH
        vendor/bitmap)

include_directories( ${BITMAP_PATH} )

set(SOURCE_CODE
        src/main.cpp src/geometry.h src/shader.h src/io.h src/kdtree.h src/tracer.h src/colors.h src/image.h src/material.h)

add_executable(gallifrey
        ${SOURCE_CODE} )
