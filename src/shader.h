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
inline Vec get_phong_shading_vector(const Face &f, const Vec &pos, const Scene &s)
{
    int j;
    float alpha, beta, gamma;
    locate(f, pos, s, j, alpha, beta, gamma);
    Vec ret = (s.vn_array[f.get_elem_idxVn(0)] * alpha + s.vn_array[f.get_elem_idxVn(j)] * beta + s.vn_array[f.get_elem_idxVn(j + 1)] * gamma);
    return ret.norm();
}

#endif //GALLIFREY_SHADER_H
