//
// Created by zihao on 2017/3/26.
//

#ifndef GALLIFREY_TRACING_H_H
#define GALLIFREY_TRACING_H_H


#include "geometry.h"
#include "shader.h"

const int max_depth = 10;


namespace light
{
    const Vec Ia(.3, .3, .3);
    const int samps = 1;
    const float n = 1.7;
}

/*
 * To calculate the ambient light at the given position.
 */
inline Vec ambient_light(const Vec &pos, const Vec &ka)
{
    return light::Ia * ka;
}

/*
 * To calculate the diffuse light at the given position.
 * pos: position
 * kd: diffusion coefficient.
 * N: normal vector
 */
inline Vec diffuse_light(const Vec &pos, const Vec &kd, const Vec &N, const Scene &s)
{
    Vec ret(0, 0, 0);
    for (int i = 0; i < s.size_il; ++i)
    {
        /*
         * TODO
         * However, there should be some codes related to intersection here, but I haven't finished them.
         */
        float cosine = N.dot((s.il_array[i] - pos).norm());
        if (cosine > 0)
            ret = ret + kd * cosine;
    }
    ret = ret * (1. / s.size_il);
    assert(ret.x >= 0 && ret.y >= 0 && ret.z >= 0);
    return ret;
}

/*
 * To calculate the specular light at the given position.
 * pos: position
 * ks: specular coefficient.
 * N: normal vector
 * V: view vector
 */
inline Vec specular_light(const Vec &pos, const Vec &ks, const Vec &N, Vec V, const Scene &s)
{
    Vec ret(0, 0, 0);
    for (int i = 0; i < s.size_il; ++i)
    {
        /*
         * TODO
         * However, there should be some codes related to intersection here, but I haven't finished them.
         */
        Vec L = (s.il_array[i] - pos).norm();
        Vec R = N * ((L.dot(N)) * 2) - L;
        float cosine = V.norm().dot(R);
        if (cosine > 0)
            ret = ret + ks * pow(cosine, light::n);
    }
    ret = ret * (1. / s.size_il);
    assert(ret.x >= 0 && ret.y >= 0 && ret.z >= 0);
    return ret;
}

/*
 * To calculate the refract_light at the given position.
 * pos: position
 */
inline Vec refract_light(const Vec &pos)
{
    // TODO
    return Vec(0, 0, 0);
}

/*
 * Ray Tracing part.
 */
Vec radiance(const Ray &r, int depth, const Scene &s, int E = 1)
{
    if (depth > max_depth)
    {
        return Vec(1, 1, 1);
    }
    float t;
    int id;
    if (naive_intersect(r, t, id, s))
    {
        Vec des = r.o + r.d * t;
        /*
         * TODO
         * ray casting -> ray tracing.
         */
        Vec N(get_phong_shading_vector(s.f_array[id], des, s));
        //if (s.fn_array[id].dot(r.d) < 0)
        //    N.set_coordinate(s.fn_array[id].x, s.fn_array[id].y, s.fn_array[id].z);
        //else
        //    N.set_coordinate(-s.fn_array[id].x, -s.fn_array[id].y, -s.fn_array[id].z);

        return ambient_light(des, Vec(.3, .3, .3)) +
               diffuse_light(des, Vec(.6, .6, .6), N, s) +
               specular_light(des, Vec(.7, .7, .7), N, Vec(-r.d.x, -r.d.y, -r.d.z), s);
    }
    else
    {
        return Vec(0, 0, 0);
    }
}

#endif //GALLIFREY_TRACING_H_H
