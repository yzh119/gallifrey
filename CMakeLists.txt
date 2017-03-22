cmake_minimum_required(VERSION 3.5)
project(gallifrey)
set(CMAKE_CXX_STANDARD 11)

include_directories( ${PROJECT_SOURCE_DIR}/vendor/ply )

#find_package(Boost REQUIRED)
#find_package(OpenGL REQUIRED)

#include_directories(${OPENGL_INCLUDE_DIR} ${BOOST_INCLUDE_DIR})

set(SOURCE_CODE
        src/main.cpp src/geometry.h src/shader.h src/io.h)

add_executable(gallifrey
        ${SOURCE_CODE})

