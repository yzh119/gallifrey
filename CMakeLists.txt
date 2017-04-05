cmake_minimum_required(VERSION 3.5)
project(gallifrey)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
if (WIN32)
set(WIN_OPENCV_INCLUDE_PATH "C:\\\\opencv\\\\install\\\\include")
set(WIN_OPENCV_LIB_PATH "C:\\\\opencv\\\\install\\\\x86\\\\mingw\\\\lib")
include_directories(${WIN_OPENCV_INCLUDE_PATH})
set(OpenCV_LIBS
        ${WIN_OPENCV_LIB_PATH}\\\\libopencv_core2413.dll.a
        ${WIN_OPENCV_LIB_PATH}\\\\libopencv_highgui2413.dll.a
        ${WIN_OPENCV_LIB_PATH}\\\\libopencv_imgproc2413.dll.a)
else()
find_package( OpenCV REQUIRED )
endif()

if (CMAKE_BUILD_TYPE MATCHES RELEASE)
set(EXECUTABLE_OUTPUT_PATH bin)
else()
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
endif()

include_directories( vendor/bitmap vendor/concurrentqueue)

set(SOURCE_CODE
        src/main.cpp src/geometry.h src/shader.h src/io.h src/kdtree.h src/tracing.h src/colors.h src/image.h src/aabb.h src/texture.h)

add_executable(gallifrey
        ${SOURCE_CODE})

target_link_libraries(gallifrey
        pthread ${OpenCV_LIBS})