# Gallifrey
Course assignment(midterm) of CS230.

To implement <del>an efficient</del> Ray Tracing algorithm on CPU.

Default resolution is 640 x 480, if you would like to use other configurations, please refer to `src/image.h` and change the value of `height` and `width`.

If you would like to run the code on CPU with more then 4 cores, please refer to `src/main.cpp` and change the value of `num_workers`.

## Features

- Phong Shading
- Anti Aliasing
- Global / Local illumination
- SAH KDTree
- Multi Threading
- Monte Carlo Ray Tracing
- Texture

## Requirements

- gcc/g++ 4.8+, std-c++11
- ConcurrentQueue(https://github.com/cameron314/concurrentqueue.git)
- Bitmap(https://github.com/ArashPartow/bitmap.git)

## Compile & Run
This project is cross-platform (Windows, Unix/Linux). But it's not recommended to compile & run on Windows cause multi-threading is disabled, thus rendering may take a long time.

For Unix/Linux users:

    git clone https://github.com/yzh119/gallifrey.git --recurse-submodules
    cd gallifrey/
    cmake -DCMAKE_BUILD_TYPE=RELEASE .
    make && cd bin
    ./gallifrey [-anti_aliasing] [-shadow] [-global]
    cd ../out

For Windows users(using Powershell, with `git`, `mingw` in your `PATH` environment variable):

    git clone https://github.com/yzh119/gallifrey.git --recurse-submodules
    cd gallifrey/
    cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=RELEASE .
    mingw32-make install
    cd bin
    ./gallifrey [-anti_aliasing] [-shadow] [-global]
    cd ../out

## Milestone
- [x] Surface Area Heuristic KD-Tree
- [x] Anti Aliasing
- [x] Phong Shader
- [x] Ray Casting (Local illumination)
- [ ] Ray Tracing (Global illumination)
- [ ] Soft Shadow
- [ ] Transparent
- [ ] Color Blending
- [x] Texture
- [x] Multi-threading

## Statistics

## Demo
![teapot](demo/teapot.bmp)
![airboat](demo/airboat.bmp)

## Reference
- An Integrated Introduction to Computer Graphics and Geometric Modeling. Ron Goldman
- Global Illumination in 99 lines of C++: http://www.kevinbeason.com/smallpt/
- SAH kd-tree: http://dcgi.felk.cvut.cz/home/havran/ARTICLES/ingo06rtKdtree.pdf
- About how to load textures: https://learnopengl-cn.readthedocs.io/zh/latest/01%20Getting%20started/06%20Textures/
- Yuxin Wu's repository: https://github.com/ppwwyyxx/Ray-Tracing-Engine
