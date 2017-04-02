//
// Created by zihao on 2017/3/26.
//

#ifndef GALLIFREY_TRACING_H_H
#define GALLIFREY_TRACING_H_H


#include "geometry.h"
#include "shader.h"

const int max_depth = 2;
extern bool enable_shadow;

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
        float cosine = N.dot((s.il_array[i] - pos).norm());
        if (!enable_shadow)
        {
            if (cosine > 0)
                ret = ret + kd * cosine;
        }
        else
        {
            if (cosine > 0 && !oriented_segment_intersect(pos, s.il_array[i], s))
                ret = ret + kd * cosine;
        }

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
        Vec L = (s.il_array[i] - pos).norm();
        Vec R = N * ((L.dot(N)) * 2) - L;
        float cosine = V.norm().dot(R);
        if (!enable_shadow)
        {
            if (cosine > 0)
                ret = ret + ks * pow(cosine, light::n);
        }
        else
        {
            if (cosine > 0 && !oriented_segment_intersect(pos, s.il_array[i], s))
                ret = ret + ks * pow(cosine, light::n);
        }
    }
    ret = ret * (1. / s.size_il);
    assert(ret.x >= 0 && ret.y >= 0 && ret.z >= 0);
    return ret;
}

/*
 * Local illumination
 */
inline Vec local_ill(const Ray &r, const Scene &s, int E = 1)
{
    float t;
    int id;
    if (naive_intersect(r, t, id, s))
    {
        Vec des = r.o + r.d * t;
        Vec N(get_phong_shading_vector(s.f_array[id], des, s));
        return ambient_light(des, Vec(.2, .2, .2)) +
               diffuse_light(des, Vec(.4, .4, .4), N, s) +
               specular_light(des, Vec(.4, .4, .4), N, Vec(-r.d.x, -r.d.y, -r.d.z), s);
    }
    else
    {
        return Vec(0, 0, 0);
    }
}

Vec global_ill(const Ray &r, int depth, const Scene &s)
{
    if (depth > max_depth)
        return Vec(0, 0, 0);

    float t;
    int id;
    if (naive_intersect(r, t, id, s))
    {
        Vec des = r.o + r.d * t;
        // TODO
    } else
    {
        return Vec(0, 0, 0);
    }
}


#endif //GALLIFREY_TRACING_H_H
