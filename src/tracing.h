//
// Created by zihao on 2017/3/26.
//

#ifndef GALLIFREY_TRACING_H_H
#define GALLIFREY_TRACING_H_H


#include "geometry.h"
#include "shader.h"
#include "texture.h"
#include "kdtree.h"

const int max_depth = 2;
extern bool enable_shadow;

namespace light
{
    const Vec Ia(.5, .5, .5);
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
        Vec N(get_phong_shading_vector(s.f_array[id], des, s));
        return (ambient_light(des, s.f_array[id].material.ka) +
               diffuse_light(des, s.f_array[id].material.kd, N, s) +
               specular_light(des, s.f_array[id].material.ks, N, Vec(-r.d.x, -r.d.y, -r.d.z), s)) * get_texture_at_pos(s.f_array[id], des, s);
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
        Face &f = s.f_array[id];
        Vec c = f.material.c;
        float p = c.x > c.y && c.x > c.z ? c.x : c.y > c.z ? c.y : c.z;
        if (++depth > 5)
        {
            if (erand() < p)
                c = c * (1. / p);
            else
                return f.material.e;
        }
        Vec n = s.fn_array[id],
            nl = (n.dot(r.d) < 0) ? n: Vec(-n.x, -n.y, -n.z);

        // Interpolate normal vector.
        if (f.area < s.area * 1e-2)
        {
            n = get_phong_shading_vector(s.f_array[id], des, s);
            nl = (n.dot(r.d) < 0) ? n: Vec(-n.x, -n.y, -n.z);
        }

        // The following codes are copied from http://www.kevinbeason.com/smallpt/
        switch (f.material.refl) {
            case DIFF:              // Diffusion
            {
                double r1 = 2 * M_PI * erand(), r2 = erand(), r2s = sqrt(r2);
                Vec w = nl, u = ((fabs(w.x) > .1 ? Vec(0, 1) : Vec(1)) % w).norm(), v = w % u;
                Vec d = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).norm();
                return get_texture_at_pos(f, des, s) * (f.material.e + c * global_ill(Ray(des, d), depth, s));
            }
            case SPEC:              // Specular
            {
                return get_texture_at_pos(f, des, s) * (f.material.e + c * global_ill(Ray(des, r.d - n * 2 * n.dot(r.d)), depth, s));
            }
            case REFR:              // Reflection
            {
                Ray reflRay(des, r.d - n * 2 * n.dot(r.d));     // Ideal dielectric REFRACTION
                bool into = n.dot(nl)>0;                // Ray from outside going in?
                double nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = r.d.dot(nl), cos2t;
                if ((cos2t = 1 - nnt * nnt * (1 - ddn * ddn)) < 0)    // Total internal reflection
                    return get_texture_at_pos(f, des, s) * (f.material.e + c * global_ill(reflRay, depth, s));
                Vec tdir = (r.d * nnt - n * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).norm();
                double a = nt - nc, b = nt + nc, R0 = a * a / (b * b), c1 = 1-(into?-ddn:tdir.dot(n));
                double Re = R0 + (1 - R0) * c1 * c1 * c1 * c1 * c1, Tr = 1 - Re, P = .25 + .5 * Re, RP = Re / P, TP = Tr / (1 - P);
                return get_texture_at_pos(f, des, s) * (f.material.e + c * (depth > 2 ? (erand() < P ? global_ill(reflRay, depth, s) * RP: global_ill(Ray(des, tdir), depth, s) * TP):
                                           global_ill(reflRay, depth, s) * Re + global_ill(Ray(des, tdir), depth, s)*Tr)); // Russian roulette
            }
        }
    } else
    {
        return Vec();
    }
}

/*
Vec radiance(const Ray &r, int depth, unsigned short *Xi){
    double t;                               // distance to intersection
    int id=0;                               // id of intersected object
    if (!intersect(r, t, id)) return Vec(); // if miss, return black
    const Sphere &obj = spheres[id];        // the hit object
    Vec x=r.o+r.d*t, n=(x-obj.p).norm(), nl=n.dot(r.d)<0?n:n*-1, f=obj.c;
    double p = f.x>f.y && f.x>f.z ? f.x : f.y>f.z ? f.y : f.z; // max refl
    if (++depth>5) if (erand48(Xi)<p) f=f*(1/p); else return obj.e; //R.R.

    Ray reflRay(x, r.d-n*2*n.dot(r.d));     // Ideal dielectric REFRACTION
    bool into = n.dot(nl)>0;                // Ray from outside going in?
    double nc=1, nt=1.5, nnt=into?nc/nt:nt/nc, ddn=r.d.dot(nl), cos2t;
    if ((cos2t=1-nnt*nnt*(1-ddn*ddn))<0)    // Total internal reflection
        return obj.e + f.mult(radiance(reflRay,depth,Xi));
    Vec tdir = (r.d*nnt - n*((into?1:-1)*(ddn*nnt+sqrt(cos2t)))).norm();
    double a=nt-nc, b=nt+nc, R0=a*a/(b*b), c = 1-(into?-ddn:tdir.dot(n));
    double Re=R0+(1-R0)*c*c*c*c*c,Tr=1-Re,P=.25+.5*Re,RP=Re/P,TP=Tr/(1-P);
    return obj.e + f.mult(depth>2 ? (erand48(Xi)<P ?   // Russian roulette
                                     radiance(reflRay,depth,Xi)*RP:radiance(Ray(x,tdir),depth,Xi)*TP) :
                          radiance(reflRay,depth,Xi)*Re+radiance(Ray(x,tdir),depth,Xi)*Tr);
}*/


#endif //GALLIFREY_TRACING_H_H
