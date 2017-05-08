//
// Created by zihao on 2017/3/23.
//

#ifndef GALLIFREY_SHADER_H
#define GALLIFREY_SHADER_H

#include <cstdio>
#include "geometry.h"

/*
 * To interpolate the normal vector.
 */
inline Vec get_interpolate_vector(const Triangle &t, const Vec &pos)
{
    float alpha, beta, gamma;
    locate(t, pos, alpha, beta, gamma);
    Vec ret = Vec(t.vn[0][0], t.vn[0][1], t.vn[0][2]) * alpha +
            Vec(t.vn[1][0], t.vn[1][1], t.vn[1][2]) * beta +
            Vec(t.vn[2][0], t.vn[2][1], t.vn[2][2]) * gamma;
    return ret.norm();
}

#endif //GALLIFREY_SHADER_H
