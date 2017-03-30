//
// Created by zihao on 2017/3/26.
//

#ifndef GALLIFREY_TRACING_H_H
#define GALLIFREY_TRACING_H_H


#include "geometry.h"

const int max_depth = 10;

Vec radiance(const Ray &r, int depth, const Scene &s, int E = 1)
{
    //printf("%f %f %f\n", r.o.x, r.o.y, r.o.z);
    if (depth > max_depth)
    {
        return Vec(0, 0, 0);
    }
    float t;
    int id;

    if (naive_intersect(r, t, id, s))
    {
        return Vec(1, 1, 1);
    }
    else
    {
        return Vec(0, 0, 0);
    }
}

#endif //GALLIFREY_TRACING_H_H
