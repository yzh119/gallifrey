//
// Created by zhy96 on 3/30/17.
//

#ifndef GALLIFREY_AABB_H
#define GALLIFREY_AABB_H

#include "geometry.h"

class AABB
{
public:
    AABB(const Face &f, Scene *s) {
        min_x = min_y = min_z = 1e9;
        max_x = max_y = max_z = (float) -1e9;
        for (int i = 0; i < f.get_size(); ++i)
        {
            float
                    x = s->vx_array[f.get_elem_idxV(i)].x,
                    y = s->vx_array[f.get_elem_idxV(i)].x,
                    z = s->vx_array[f.get_elem_idxV(i)].x;
            if (x > max_x) max_x = x;
            if (y > max_y) max_y = y;
            if (z > max_z) max_z = z;

            if (x < min_x) min_x = x;
            if (y < min_y) min_y = y;
            if (z < min_z) min_z = z;
        }
    }
    float
            min_x, min_y, min_z,
            max_x, max_y, max_z;
};

#endif //GALLIFREY_AABB_H
