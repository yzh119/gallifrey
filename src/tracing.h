//
// Created by zihao on 2017/3/26.
//

#ifndef GALLIFREY_TRACING_H_H
#define GALLIFREY_TRACING_H_H


#include "geometry.h"

const int max_depth = 10;


namespace light
{
    const Vec Ia(0.8, 0.3, 0.4);
    const int samps = 1;
    const float n = 1.3;
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
inline Vec diffuse_light(const Vec &pos, const Vec &kd, const Vec &N)
{
    return Vec(0, 0, 0);
}

/*
 * To calculate the specular light at the given position.
 * pos: position
 * ks: specular coefficient.
 * N: normal vector
 * V: view vector
 */
inline Vec specular_light(const Vec &pos, const Vec &ks, const Vec &N, const Vec &V)
{
    return Vec(0, 0, 0);
}

/*
 * To calculate the refract_light at the given position.
 * pos: position
 */
inline Vec refract_light(const Vec &pos)
{
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
        return ambient_light(des, Vec(.5, .5, .5)) +
               diffuse_light(des, Vec(.3, .4, .5), s.fn_array[id]) +
               specular_light(des, Vec(0, 0, 0), s.fn_array[id], r.d);
    }
    else
    {
        return Vec(0, 0, 0);
    }
}

#endif //GALLIFREY_TRACING_H_H
