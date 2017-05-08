//
// Created by zihao on 2017/3/26.
//

#ifndef GALLIFREY_TRACING_H_H
#define GALLIFREY_TRACING_H_H


#include "geometry.h"
#include "interpolate.h"
#include "texture.h"
#include "kdtree.h"

extern bool enable_shadow;

namespace light
{
    const Vec Ia(.5, .5, .5);
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
                ret = ret + kd * cosine * s.li_array[i];
        }
        else
        {
            if (cosine > 0 && !high_level_oriented_segment_intersect(pos, s.il_array[i], s))
                ret = ret + kd * cosine * s.li_array[i];
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
                ret = ret + ks * pow(cosine, light::n) * s.li_array[i];
        }
        else
        {
            if (cosine > 0 && !high_level_oriented_segment_intersect(pos, s.il_array[i], s))
                ret = ret + ks * pow(cosine, light::n) * s.li_array[i];
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
    float t = 1e8;
    int id;
    if (high_level_intersect(r, t, id, s))
    {
        Vec des = r.o + r.d * t;
        Vec N(get_interpolate_vector(s.t_array[id], des));
        return (ambient_light(des, s.t_array[id].m->ka) +
               diffuse_light(des, s.t_array[id].m->kd, N, s) +
               specular_light(des, s.t_array[id].m->ks, N, Vec(-r.d.x, -r.d.y, -r.d.z), s)) * get_texture_at_pos(s.t_array[id], des);
    }
    else
    {
        return Vec();
    }
}

Vec global_ill(const Ray &r, int depth, const Scene &s)
{
    float t = 1e8;
    int id;
    if (high_level_intersect(r, t, id, s))
    {
        Vec des = r.o + r.d * t;
        Triangle &f = s.t_array[id];
        Vec c = f.m->c;
        float p = c.x < c.y ? c.x : c.y;
        p = p < c.z ? p: c.z;
        if (++depth > 5)
            if (erand() < p)
                c = c * (1. / p);
            else return f.m->e;

        if (depth > 10)
            return f.m->e;

        Vec n = Vec(f.fn[0], f.fn[1], f.fn[2]),
            nl = (n.dot(r.d) < 0) ? n: Vec(-n.x, -n.y, -n.z);

        // Interpolate normal vector.
        if (f.area < s.area * 1e-2)
        {
            n = get_interpolate_vector(f, des);
            nl = (n.dot(r.d) < 0) ? n: Vec(-n.x, -n.y, -n.z);
        }

        // The following codes are copied from http://www.kevinbeason.com/smallpt/
        switch (f.m->refl) {
            case DIFF:              // Diffusion
            {
                double r1 = 2 * M_PI * erand(), r2 = erand(), r2s = sqrt(r2);
                Vec w = nl, u = ((fabs(w.x) > .1 ? Vec(0, 1) : Vec(1)) % w).norm(), v = w % u;
                Vec d = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).norm();
                return get_texture_at_pos(f, des) * (f.m->e + c * global_ill(Ray(des, d), depth, s));
            }
            case SPEC:              // Specular
            {
                return get_texture_at_pos(f, des) * (f.m->e + c * global_ill(Ray(des, r.d - n * 2 * n.dot(r.d)), depth, s));
            }
            case REFR:              // Reflection
            {
                Ray reflRay(des, r.d - n * 2 * n.dot(r.d));     // Ideal dielectric REFRACTION
                bool into = n.dot(nl)>0;                // Ray from outside going in?
                double nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = r.d.dot(nl), cos2t;
                if ((cos2t = 1 - nnt * nnt * (1 - ddn * ddn)) < 0)    // Total internal reflection
                    return get_texture_at_pos(f, des) * (f.m->e + c * global_ill(reflRay, depth, s));
                Vec tdir = (r.d * nnt - n * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).norm();
                double a = nt - nc, b = nt + nc, R0 = a * a / (b * b), c1 = 1-(into?-ddn:tdir.dot(n));
                double Re = R0 + (1 - R0) * c1 * c1 * c1 * c1 * c1, Tr = 1 - Re, P = .25 + .5 * Re, RP = Re / P, TP = Tr / (1 - P);
                return get_texture_at_pos(f, des) * (f.m->e +
                        c * (erand() < P ? global_ill(reflRay, depth, s) * RP: global_ill(Ray(des, tdir), depth, s) * TP)); // Russian roulette
            }
        }
    } else
    {
        return Vec();
    }
}

#endif //GALLIFREY_TRACING_H_H
