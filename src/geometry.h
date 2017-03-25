//
// Created by zihao on 2017/3/23.
//

#ifndef GALLIFREY_GEOMETRY_H
#define GALLIFREY_GEOMETRY_H

#include <array>

class Vertex
{
private:
    float x, y, z, w;
public:
    Vertex(float x = 0, float y = 0, float z = 0, float w = 0): x(x), y(y), z(z), w(w) {}
    ~Vertex() {}
};

class Vector
{
private:
    float x, y, z;
public:
    Vector(float x = 0, float y = 0, float z = 0): x(x), y(y), z(z) {}
    ~Vector() {}
};

class Face
{
private:
    int idxF[4], idxVn[4], idxVx[4];
    std::vector <int> extra_idxF, extra_idxVn, extra_idxVx;
    size_t sz;
public:
    Face() {}
    ~Face() {}
    void add_vx(int idx_f, int idx_vn, int idx_vx);
    size_t get_size() const {
        return sz;
    }
};

void Face::add_vx(int idx_f, int idx_vn, int idx_vx) {
    if (sz < 4) {
        idxF[sz] = idx_f;
        idxVn[sz] = idx_vn;
        idxVx[sz] = idx_vx;
        ++sz;
    } else {
        extra_idxF.push_back(idx_f);
        extra_idxVn.push_back(idx_vn);
        extra_idxVx.push_back(idx_vx);
        ++sz;
    }
    return;
}


#endif //GALLIFREY_GEOMETRY_H
