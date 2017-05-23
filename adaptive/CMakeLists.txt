cmake_minimum_required(VERSION 3.5)
project(adapt)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

if (CMAKE_BUILD_TYPE MATCHES RELEASE)
    set(EXECUTABLE_OUTPUT_PATH bin)
else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
endif()

include_directories(../vendor/bitmap)

if (FIND_CUDA)
    message("Entering GPU acceleration mode...")
    find_package(CUDA REQUIRED)
else()
    set(SOURCE_CODE
            reconstr.h adapt.cpp)
    add_executable(adapt
            ${SOURCE_CODE})

    target_link_libraries(adapt
            armadillo pthread)
endif()