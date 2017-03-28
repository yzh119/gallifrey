//
// Created by zihao on 2017/3/23.
//

#ifndef GALLIFREY_GEOMETRY_H
#define GALLIFREY_GEOMETRY_H

/*
 * Class Vector:
 * - vector
 * - vertex
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
    return Vec(x - b.x, y + b.y, z - b.z);
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
    int idxF[4], idxVn[4], idxVx[4];
    std::vector <int> extra_idxF, extra_idxVn, extra_idxVx;
    size_t sz;
public:
    Face() {}
    ~Face() {}
    inline void add_vx(int idx_f, int idx_vn, int idx_vx);
    size_t get_size() const
    {
        return sz;
    }
    inline int get_elem_idxF(int idx);
};

inline void Face::add_vx(int idx_f, int idx_vn, int idx_vx)
{
    if (sz < 4)
    {
        idxF[sz] = idx_f;
        idxVn[sz] = idx_vn;
        idxVx[sz] = idx_vx;
        ++sz;
    }
    else
    {
        extra_idxF.push_back(idx_f);
        extra_idxVn.push_back(idx_vn);
        extra_idxVx.push_back(idx_vx);
        ++sz;
    }
    return;
}

inline int Face::get_elem_idxF(int idx)
{
    if (idx < 4)
    {
        return idxF[idx];
    }
    idx -= 4;
    return extra_idxF[idx];
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
 * Compute the intersection of a ray and a face.
 */
float intersect_with_face(const Ray &r, const Face &f)
{
    Vec inter;
    for (size_t i = 0; i < 3; ++i)
    {

    }

    bool inside;
    inside =
    if (inside)
        return
}

bool inside_one_face(const Vec &v, const Face &f)
{

}

#endif //GALLIFREY_GEOMETRY_H
