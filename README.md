# Gallifrey
Course assignment(midterm) of CS230.

To implement <del>an efficient</del> Ray Tracing algorithm.

## Compile
It's not recommended to compile & run on windows since `pthread` is not fully developed, thus multi-threading accelerating is disabled.

    git clone https://github.com/yzh119/gallifrey.git --recurse-submodules
    cd gallifrey/
    cmake .
    make


## Report

[report](http://blog.expye.com/post/study/2017-03-25-ray-Tracer) (@ my blog)


## Requirements

- std-c++11
- ConcurrentQueue(https://github.com/cameron314/concurrentqueue.git)
- Bitmap(https://github.com/ArashPartow/bitmap.git)

## Milestone
- [ ] Phong Shader
- [ ] Ray Tracing
- [ ] Soft Shadow 
- [ ] Transmission
- [ ] Transparent
- [ ] Semitransparent
- [ ] Color Bleeding
- [ ] Texture
- [x] Multithreading

## Demo

## Reference
- Global Illumination in 99 lines of C++: http://www.kevinbeason.com/smallpt/
- SAH kd-tree: http://dcgi.felk.cvut.cz/home/havran/ARTICLES/ingo06rtKdtree.pdf
- An Integrated Introduction to Computer Graphics and Geometric Modeling. Ron Goldman
