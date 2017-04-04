//
// Created by zhy96 on 3/30/17.
//

#ifndef GALLIFREY_AABB_H
#define GALLIFREY_AABB_H

#include "geometry.h"

class Box
{
public:
    Box(const Vec &min, const Vec &max) {
        bounds[0] = min;
        bounds[1] = max;
    }
    Vec bounds[2];
    inline bool intersect_with_ray(const Ray &r, float t0, float t1) const;
};

/*
 * Optimized Method
 * From **An Efficient and Robust Ray-Box Intersection Algorithm. by Amy Williams, et al.**
 */

inline bool Box::intersect_with_ray(const Ray &r, float t0, float t1) const
{
    float tmin, tmax, tymin, tymax, tzmin, tzmax;

    tmin = (bounds[r.sign[0]].x - r.o.x) * r.inv_d.x;
    tmax = (bounds[1 - r.sign[0]].x - r.o.x) * r.inv_d.x;
    tymin = (bounds[r.sign[1]].y - r.o.y) * r.inv_d.y;
    tymax = (bounds[1 - r.sign[1]].y - r.o.y) * r.inv_d.y;

    if ((tmin > tymax) || (tymin > tmax))
        return false;

    if (tymin > tmin)
        tmin = tymin;

    if (tymax < tmax)
        tmax = tymax;

    tzmin = (bounds[r.sign[2]].z - r.o.z) * r.inv_d.z;
    tzmax = (bounds[1 - r.sign[2]].z - r.o.z) * r.inv_d.z;

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    if (tzmin > tmin)
        tmin = tzmin;

    if (tzmax < tmax)
        tmax = tzmax;

    return ((tmin < t1) && (tmax > t0));
}

inline void test_aabb()
{
    Box box(Vec(0, 0, 0), Vec(1, 1, 1));
    Ray r(Vec(0, -1, 0), Vec(1, 1.0001, 1)),
        r1(Vec(0, -1, 0), Vec(1, 0.9999, 1));
    assert(box.intersect_with_ray(r, eps, 1e8));
    assert(!box.intersect_with_ray(r1, eps, 1e8));
}

#endif //GALLIFREY_AABB_H
