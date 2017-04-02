//
// Created by zihao on 2017/3/23.
//

#ifndef GALLIFREY_SHADER_H
#define GALLIFREY_SHADER_H

#include <cstdio>
#include "geometry.h"

inline Vec get_phong_shading_vector(const Face &f, const Vec &pos, const Scene &s)
{
    float alpha = (float) (1 / 3.), beta = (float) (1 / 3.), gamma = (float) (1 / 3.);
    Vec *v_array = s.vx_array;
    Vec v(pos - v_array[f.get_elem_idxV(0)]);
    int j = -1;
    Vec v1(v_array[f.get_elem_idxV(1)] - v_array[f.get_elem_idxV(0)]),
        v2(v_array[f.get_elem_idxV(2)] - v_array[f.get_elem_idxV(0)]);
    for (int i = 1; i < f.get_size() - 1; ++i)
    {
        if ((v % v1).dot
                (v % v2) <= eps)
        {
            j = i;
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
            break;
        }
        v1 = v2;
        v2 = v_array[f.get_elem_idxV(i + 2)] - v_array[f.get_elem_idxV(0)];
    }
    assert(j != -1);
    assert(0 <= beta && beta <= 1 && 0 <= gamma && gamma <= 1 && 0 <= alpha && alpha <= 1);

    Vec ret = (s.vn_array[f.get_elem_idxVn(0)] * alpha + s.vn_array[f.get_elem_idxVn(j)] * beta + s.vn_array[f.get_elem_idxVn(j + 1)] * gamma);
    return ret.norm();
}

#endif //GALLIFREY_SHADER_H
