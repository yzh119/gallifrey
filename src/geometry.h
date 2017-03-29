//
// Created by zihao on 2017/3/23.
//

#ifndef GALLIFREY_GEOMETRY_H
#define GALLIFREY_GEOMETRY_H

#include "material.h"
#include <cassert>
#include <cmath>

const float pi = (const float) acos(-1.);

/*
 * Class Vector:
 * - Vector
 * - Vertex
 * - Color
 */
class Vec
{
private:
    float x, y, z;
public:
    Vec(float x = 0, float y = 0, float z = 0): x(x), y(y), z(z) {}
    ~Vec() {}
    void set_coordinate(float x, float y, float z);
    Vec operator + (const Vec &b) const ;
    Vec operator - (const Vec &b) const ;
    Vec operator * (float b) const ;
    Vec operator % (const Vec &b) const ;
    Vec& norm();
    float dot(const Vec &b) const ;
};


Vec &Vec::norm()
{
    return *this = *this * (1 / sqrt(x * x + y * y + z * z));
}

Vec Vec::operator*(float b) const
{
    return Vec(b * x, b * y, b * z);
}

Vec Vec::operator+(const Vec &b) const
{
    return Vec(x + b.x, y + b.y, z + b.z);
}

Vec Vec::operator-(const Vec &b) const
{
    return Vec(x - b.x, y - b.y, z - b.z);
}

float Vec::dot(const Vec &b) const
{
    return x * b.x + y * b.y + z * b.z;
}

Vec Vec::operator%(const Vec &b) const
{
    return Vec(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x);
}

void Vec::set_coordinate(float x, float y, float z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

/*
 * Class Ray
 * o: origin
 * d: direction
 */
class Ray
{
public:
    Ray(const Vec &o, const Vec &d): o(o), d(d) {}
    ~Ray() {}

    Vec d;
    Vec o;
};

class Face
{
private:
    Material material;
    int idxV[4], idxVn[4], idxVx[4];                            // Index array of one face (face array, )
    std::vector <int> extra_idxV, extra_idxVn, extra_idxVx;
    size_t sz;
public:
    Face()
    {
        sz = 0;
    }
    ~Face() {}
    inline void add_vx(int idx_v, int idx_vn, int idx_vx);
    size_t get_size() const
    {
        return sz;
    }
    inline int get_elem_idxV(int idx)const;
};

float eps = 1e-6;

inline void Face::add_vx(int idx_v, int idx_vn, int idx_vx)
{
    if (sz < 4)
    {
        idxV[sz] = idx_v;
        idxVn[sz] = idx_vn;
        idxVx[sz] = idx_vx;
        ++sz;
    }
    else
    {
        extra_idxV.push_back(idx_v);
        extra_idxVn.push_back(idx_vn);
        extra_idxVx.push_back(idx_vx);
        ++sz;
    }
    return;
}

inline int Face::get_elem_idxV(int idx)const {
    if (idx >= sz) idx -= sz;
    if (idx < 4)
    {
        return idxV[idx];
    }
    idx -= 4;
    return extra_idxV[idx];
}

/*
 * INTERSECTS RAY WITH SCENE
 * Brute Force algorithm.
 */
inline bool naive_intersect(const Ray &r, double &t, int &id)
{
    // TODO
    return false;
}

/*
 * Cross function.
 */
inline bool cross(const Vec &a, const Vec &b, const Vec &c)
{
    return (a % b).dot(a % c) < -eps;
}

/*
 * Compute the intersection of a ray and a face.
 * If not intersect, return -1;
 * Else return the distance between origin of the ray and the intersection.
 */
inline float intersect_with_face(const Ray &r, const Face &f, Vec *v_list)
{
    Vec v1 = v_list[f.get_elem_idxV(1)] - v_list[f.get_elem_idxV(0)],
        v2 = v_list[f.get_elem_idxV(2)] - v_list[f.get_elem_idxV(1)];
    Vec norm_vec = v1 % v2;
    norm_vec.norm();

    float denom = norm_vec.dot(r.d),
          numer = norm_vec.dot(v_list[f.get_elem_idxV(0)] - r.o);
    if (fabs(denom) < eps)
    {
        return (float) -1.;         // Case 1: parallel
    }
    else
    {
        float dis = numer / denom;

        int cnt = 0;
        Vec inter = r.o + r.d * dis;
        Vec rand_vec = v1 * 1e4;

        for (int i = 0; i < f.get_size(); ++i)
        {
            if (cross(rand_vec, v_list[f.get_elem_idxV(i)] - inter, v_list[f.get_elem_idxV(i + 1)] - inter) &&
                    cross(v_list[f.get_elem_idxV(i + 1)] - v_list[f.get_elem_idxV(i)], v_list[f.get_elem_idxV(i)] - inter, v_list[f.get_elem_idxV(i)] - inter - rand_vec))
                ++cnt;
        }

        if (cnt % 2 == 1)
        {
            return dis;             // Case 2: intersect
        }
        else
        {
            return -1;              // Case 3: not parallel, but intersection is outside the polygon.
        }
    }
}

/*
 * Test the accuracy of intersection.
 */
void test_intersection()
{
    Vec v[4];
    v[0].set_coordinate(0, 0, 0);
    v[1].set_coordinate(10, 0, 0);
    v[2].set_coordinate(0, 10, 0);
    v[3].set_coordinate(-10, 10, 0);
    Face f;
    for (int i = 0; i < 3; ++i)
        f.add_vx(i, 0, 0);
    Ray r(Vec(2, 2, 2), Vec(0, 0, -1));
    assert(fabs(intersect_with_face(r, f, v) - 2.) < eps);
    Ray r1(Vec(-2, 4, 2), Vec(0, 0, -1));
    assert(fabs(intersect_with_face(r1, f, v) + 1.) < eps);
    f.add_vx(3, 0, 0);
    Ray r2(Vec(-2, 4, 2), Vec(0, 0, -1));
    assert(fabs(intersect_with_face(r2, f, v) - 2.) < eps);
}

#endif //GALLIFREY_GEOMETRY_H
