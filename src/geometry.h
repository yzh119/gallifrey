//
// Created by zihao on 2017/3/23.
//

#ifndef GALLIFREY_GEOMETRY_H
#define GALLIFREY_GEOMETRY_H

#include <cassert>
#include <cmath>
#include <opencv2/opencv.hpp>
inline float erand()
{
#ifdef __linux__
    static unsigned short Xi[3] = {0, 0, 0};
    Xi[2] += (Xi[1] += ++Xi[0] == 0) == 0;
    return (float) erand48(Xi);
#endif
#ifdef _WIN32
    return (float) (1. * rand() / RAND_MAX);
#endif
}

/*
 * Class Vector:
 * - Vector
 * - Vertex
 * - Color
 */
class Vec
{
public:
    Vec(float x = 0, float y = 0, float z = 0): x(x), y(y), z(z) {}
    Vec(const Vec &vec)
    {
        x = vec.x;
        y = vec.y;
        z = vec.z;
    }
    ~Vec() {}
    inline void set_coordinate(float x, float y, float z);
    inline Vec operator + (const Vec &b) const ;
    inline Vec operator - (const Vec &b) const ;
    inline Vec operator * (const Vec &b) const ;
    inline Vec operator * (float b) const ;
    inline Vec operator % (const Vec &b) const ;
    inline Vec& norm();
    inline float dot(const Vec &b) const ;

    float x;
    float y;
    float z;
};

inline Vec &Vec::norm()
{
    return *this = *this * (1 / sqrt(x * x + y * y + z * z));
}

inline Vec Vec::operator*(float b) const
{
    return Vec(b * x, b * y, b * z);
}

inline Vec Vec::operator+(const Vec &b) const
{
    return Vec(x + b.x, y + b.y, z + b.z);
}

inline Vec Vec::operator-(const Vec &b) const
{
    return Vec(x - b.x, y - b.y, z - b.z);
}

inline float Vec::dot(const Vec &b) const
{
    return x * b.x + y * b.y + z * b.z;
}

inline Vec Vec::operator%(const Vec &b) const
{
    return Vec(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x);
}

inline void Vec::set_coordinate(float x, float y, float z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

inline Vec Vec::operator*(const Vec &b) const {
    return Vec(x * b.x, y * b.y, z * b.z);
}

enum refl_t {DIFF, SPEC, REFR};

/*
 * Class Material
 */
class Material
{
public:
    Vec ka, kd, ks;         // k_ambient, k_diffusion, k_specular, used for local illumination.
    Vec e, c;               // emission, color, used for global illumination
    cv::Mat *image;
    refl_t refl;
    Material(const Vec &ka, const Vec &kd, const Vec &ks, cv::Mat *img = nullptr, const Vec &e = Vec(), const Vec &c = Vec(), refl_t refl = DIFF):
            ka(ka), kd(kd), ks(ks), image(img), e(e), c(c), refl(refl) {}
};

/*
 * Class Ray
 * o: origin
 * d: direction
 */
class Ray
{
private:
    const float eps = 1e-8;
public:
    Ray(const Vec &o, const Vec &d): o(o), d(d) {
        if (d.x >= 0 && d.x < eps) this->d.x += eps;
        if (d.x <= 0 && d.x > -eps) this->d.x -= eps;
        if (d.y >= 0 && d.y < eps) this->d.y += eps;
        if (d.y <= 0 && d.y > -eps) this->d.y -= eps;
        if (d.z >= 0 && d.z < eps) this->d.z += eps;
        if (d.z <= 0 && d.z > -eps) this->d.z -= eps;
        inv_d = Vec((float) (1. / d.x), (float) (1. / d.y), (float) (1. / d.z));
        sign[0] = (d.x < 0);
        sign[1] = (d.y < 0);
        sign[2] = (d.z < 0);
    }
    Ray(const Ray &ray)
    {
        d = ray.d;
        o = ray.o;
        inv_d = ray.inv_d;
        for (int i = 0; i < 3; ++i)
            sign[i] = ray.sign[i];
    }
    ~Ray() {}

    Vec d, inv_d;
    Vec o;
    int sign[3];
};

/*
 * Class Face
 */
class Face
{
private:
    int idxV[4], idxVn[4], idxVt[4];                            // Index array of one face (face array, )
    std::vector <int> extra_idxV, extra_idxVn, extra_idxVt;
    size_t sz;
public:
    Material material;
    float area;
    Face():
        material(Vec(0, 0, 0),
                 Vec(0, 0, 0),
                 Vec(0, 0, 0))
    {
        sz = 0;
    }
    ~Face() {}
    inline void add_vx(int idx_v, int idx_vn, int idx_vt);
    inline void modify_vn(int key, int idx_vn);
    inline void modify_vt(int key, int idx_vt);
    inline size_t get_size() const;
    inline int get_elem_idxV(int idx)const;
    inline int get_elem_idxVt(int idx)const;
    inline int get_elem_idxVn(int idx)const;
    inline void set_ka(const Vec &ka);
    inline void set_kd(const Vec &kd);
    inline void set_ks(const Vec &ks);
    inline void set_e(const Vec &e);
    inline void set_c(const Vec &c);
    inline void set_refl(refl_t refl);
};

float eps = 1e-6;

inline void Face::add_vx(int idx_v, int idx_vn, int idx_vt)
{
    if (sz < 4)
    {
        idxV[sz] = idx_v;
        idxVn[sz] = idx_vn;
        idxVt[sz] = idx_vt;
        ++sz;
    }
    else
    {
        extra_idxV.push_back(idx_v);
        extra_idxVn.push_back(idx_vn);
        extra_idxVt.push_back(idx_vt);
        ++sz;
    }
    return;
}

inline int Face::get_elem_idxV(int idx)const {
    assert(idx >= 0);
    while (idx >= sz) idx -= sz;
    if (idx < 4)
    {
        return idxV[idx];
    }
    idx -= 4;
    return extra_idxV[idx];
}

inline int Face::get_elem_idxVn(int idx) const {
    assert(idx >= 0);
    while (idx >= sz) idx -= sz;
    if (idx < 4)
    {
        return idxVn[idx];
    }
    idx -= 4;
    return extra_idxVn[idx];
}

inline int Face::get_elem_idxVt(int idx) const {
    assert(idx >= 0);
    while (idx >= sz) idx -= sz;
    if (idx < 4)
    {
        return idxVt[idx];
    }
    idx -= 4;
    return extra_idxVt[idx];
}

inline size_t Face::get_size() const {
    return sz;
}

inline void Face::set_ka(const Vec &ka) {
    material.ka = ka;
}

inline void Face::set_kd(const Vec &kd) {
    material.kd = kd;
}

inline void Face::set_ks(const Vec &ks) {
    material.ks = ks;
}

inline void Face::modify_vn(int key, int idx_vn) {
    assert(key >= 0 && key < sz);
    if (key < 4)
    {
        idxVn[key] = idx_vn;
    }
    else
    {
        extra_idxVn[key - 4] = idx_vn;
    }
    return;
}

inline void Face::modify_vt(int key, int idx_vt) {
    assert(key >= 0 && key < sz);
    if (key < 4)
    {
        idxVt[key] = idx_vt;
    }
    else
    {
        extra_idxVt[key - 4] = idx_vt;
    }
    return;
}

inline void Face::set_e(const Vec &e) {
    material.e = e;
    return;
}

inline void Face::set_c(const Vec &c) {
    material.c = c;
    return;
}

void Face::set_refl(refl_t refl) {
    material.refl = refl;
    return;
}

/*
 * Cross function.
 */
inline bool cross(const Vec &a, const Vec &b, const Vec &c)
{
    return (a % b).dot(a % c) < -eps;
}

/*
 * Test the accuracy of intersection.
 */
/*
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
}*/

struct Triangle{
    float vx[3][3];
    float vn[3][3];
    float vt[3][2];
    float fn[3];
    float area;
    Material *m;
};

struct Scene
{
    Face *f_array;
    Triangle *t_array;
    Vec *vn_array;
    Vec *vx_array;
    Vec *vt_array;
    Vec *fn_array;
    Vec *il_array;
    Vec *li_array;
    size_t size_f, size_vn, size_vx, size_vt, size_il, size_tr;
    float area;
};

/*
 * Compute the intersection of a ray with a face.
 * If not intersect, return -1;
 * Else return the distance between origin of the ray and the intersection.
 */
inline float intersect_with_face(const Ray &r, const Triangle &t)
{
    Vec
            v0(t.vx[1][0] - t.vx[0][0], t.vx[1][1] - t.vx[0][1], t.vx[1][2] - t.vx[0][2]),
            v1(t.vx[2][0] - t.vx[0][0], t.vx[2][1] - t.vx[0][1], t.vx[2][2] - t.vx[0][2]),
            v2(t.vx[0][0], t.vx[0][1], t.vx[0][2]);
    Vec norm_vec = v0 % v1;
    float
            denom = norm_vec.dot(r.d),
            numer = norm_vec.dot(v2 - r.o);
    if (fabs(denom) < eps)
        return -1;         // Case 1: parallel
    else
    {
        float dis = numer / denom;
        v2 = r.o + r.d * dis - v2;

        float
                dot00 = v0.dot(v0),
                dot01 = v0.dot(v1),
                dot02 = v0.dot(v2),
                dot11 = v1.dot(v1),
                dot12 = v1.dot(v2);

        float inver = 1 / (dot00 * dot11 - dot01 * dot01),
            u = (dot11 * dot02 - dot01 * dot12) * inver,
            v = (dot00 * dot12 - dot01 * dot02) * inver;

        if (u >= 0 && u <= 1 && v >= 0 && v <= 1 && u + v <= 1)
            return dis;
        else
            return -1;
    }
}

/*
 * To locate the position of a vertex in the face, and derive alpha, beta, gamma.
 */
inline void locate(const Triangle &t, const Vec &pos, float &alpha, float &beta, float &gamma)
{
    alpha = beta = gamma = (float) (1 / 3.);
    Vec v(pos - Vec(t.vx[0][0], t.vx[0][1], t.vx[0][2])),
            v1(Vec(t.vx[1][0], t.vx[1][1], t.vx[1][2]) - Vec(t.vx[0][0], t.vx[0][1], t.vx[0][2])),
            v2(Vec(t.vx[2][0], t.vx[2][1], t.vx[2][2]) - Vec(t.vx[0][0], t.vx[0][1], t.vx[0][2]));
    if (fabs(v1.x * v2.y - v1.y * v2.x) > eps)
    {
        beta = (v.x * v2.y - v.y * v2.x) / (v1.x * v2.y - v1.y * v2.x);
        gamma = -(v.x * v1.y - v.y * v1.x) / (v1.x * v2.y - v1.y * v2.x);
    }
    else if (fabs(v1.z * v2.y - v1.y * v2.z) > eps)
    {
        beta = (v.z * v2.y - v.y * v2.z) / (v1.z * v2.y - v1.y * v2.z);
        gamma = -(v.z * v1.y - v.y * v1.z) / (v1.z * v2.y - v1.y * v2.z);
    } else
    {
        beta = (v.z * v2.x - v.x * v2.z) / (v1.z * v2.x - v1.x * v2.z);
        gamma = -(v.z * v1.x - v.x * v1.z) / (v1.z * v2.x - v1.x * v2.z);
    }
    alpha = 1 - beta - gamma;
    if (!(0 <= beta && beta <= 1 && 0 <= gamma && gamma <= 1 && 0 <= alpha && alpha <= 1))
    {
        alpha = beta = gamma = (float) (1 / 3.);
    }
}

#endif //GALLIFREY_GEOMETRY_H
