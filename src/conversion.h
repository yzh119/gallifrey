//
// Created by zhy96 on 5/7/17.
//

#ifndef GALLIFREY_CONVERSION_H
#define GALLIFREY_CONVERSION_H

#include "geometry.h"

void convert_poly_to_triangle(Vec *vx, Vec *vn, Vec *vt, Vec *fn, Face *f, Triangle *t, const size_t &f_size, size_t &t_size)
{
    fprintf(stderr, "Reconstructing face...\n");
    for (int i = 0; i < f_size; ++i)
    {
        int id_1_vx = f[i].get_elem_idxV(0),
            id_1_vn = f[i].get_elem_idxVn(0),
            id_1_vt = f[i].get_elem_idxVt(0);
        for (size_t j = 2; j < f[i].get_size(); ++j)
        {
            int id_2_vx = f[i].get_elem_idxV(j - 1),
                id_2_vn = f[i].get_elem_idxVn(j - 1),
                id_2_vt = f[i].get_elem_idxVt(j - 1),
                id_3_vx = f[i].get_elem_idxV(j),
                id_3_vn = f[i].get_elem_idxVn(j),
                id_3_vt = f[i].get_elem_idxVt(j);
            t[t_size].vx[0][0] = vx[id_1_vx].x;
            t[t_size].vx[0][1] = vx[id_1_vx].y;
            t[t_size].vx[0][2] = vx[id_1_vx].z;
            t[t_size].vx[1][0] = vx[id_2_vx].x;
            t[t_size].vx[1][1] = vx[id_2_vx].y;
            t[t_size].vx[1][2] = vx[id_2_vx].z;
            t[t_size].vx[2][0] = vx[id_3_vx].x;
            t[t_size].vx[2][1] = vx[id_3_vx].y;
            t[t_size].vx[2][2] = vx[id_3_vx].z;

            t[t_size].vn[0][0] = vn[id_1_vn].x;
            t[t_size].vn[0][1] = vn[id_1_vn].y;
            t[t_size].vn[0][2] = vn[id_1_vn].z;
            t[t_size].vn[1][0] = vn[id_2_vn].x;
            t[t_size].vn[1][1] = vn[id_2_vn].y;
            t[t_size].vn[1][2] = vn[id_2_vn].z;
            t[t_size].vn[2][0] = vn[id_3_vn].x;
            t[t_size].vn[2][1] = vn[id_3_vn].y;
            t[t_size].vn[2][2] = vn[id_3_vn].z;

            t[t_size].vt[0][0] = vt[id_1_vt].x;
            t[t_size].vt[0][1] = vt[id_1_vt].y;
            t[t_size].vt[1][0] = vt[id_2_vt].x;
            t[t_size].vt[1][1] = vt[id_2_vt].y;
            t[t_size].vt[2][0] = vt[id_3_vt].x;
            t[t_size].vt[2][1] = vt[id_3_vt].y;

            t[t_size].m = &f[i].material;
            t[t_size].fn[0] = fn[i].x;
            t[t_size].fn[1] = fn[i].y;
            t[t_size].fn[2] = fn[i].z;
            t[t_size].area = (float) (sqrt(((vx[id_2_vx] - vx[id_1_vx]) % (vx[id_3_vx] - vx[id_1_vx])).dot(
                                (vx[id_2_vx] - vx[id_1_vx]) % (vx[id_3_vx] - vx[id_1_vx]))) / 2);
            ++t_size;
        }
    }
    fprintf(stderr, "Reconstruct successful.\n");
}

#endif //GALLIFREY_CONVERSION_H
