//
// Created by zhy96 on 3/30/17.
//

#ifndef GALLIFREY_AABB_H
#define GALLIFREY_AABB_H

#include "geometry.h"

class AABB
{
private:
    Face *f;
    float
        min_x, min_y, min_z,
        max_x, max_y, max_z;
public:
    AABB(Face *f): f(f) {
        min_x = min_y = min_z = 1e9;
        max_x = max_y = max_z = -1e9;
        for (int i = 0; i < f->get_size(); ++i)
        {
            float
                    x = f->get_elem_idxV(i).x,
                    y = f->get_elem_idxV(i).y,
                    z = f->get_elem_idxV(i).z;
            if (x > max_x) max_x = x;
            if (y > max_y) max_y = y;
            if (z > max_z) max_z = z;

            if (x < min_x) min_x = x;
            if (y < min_y) min_y = y;
            if (z < min_z) min_z = z;
        }
    }
};

#endif //GALLIFREY_AABB_H
