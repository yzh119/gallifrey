//
// Created by zhy96 on 4/2/17.
//

#ifndef GALLIFREY_TEXTURE_H
#define GALLIFREY_TEXTURE_H

#include "geometry.h"
#include <opencv2/opencv.hpp>

inline Vec get_texture_at_pos(const Face &f, Vec pos, const Scene &s)
{
    Vec col;
    if (f.material.image == nullptr)
    {
        col = Vec(1, 1, 1);
    }
    else
    {
        col = Vec(1, 1, 1);
    }
    return col;
}

#endif //GALLIFREY_TEXTURE_H
