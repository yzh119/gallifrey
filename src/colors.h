//
// Created by zihao on 2017/3/26.
// Copied from smallpt_cline_slides.
//

#ifndef GALLIFREY_COLORS_H
#define GALLIFREY_COLORS_H

#include <cmath>
#include <stdint-gcc.h>

/*
 * CLamp function. (Let upper bound be 1 and lower bound be 0)
 */
inline float clamp(float x)
{
    return x < 0 ? 0 : x > 1 ? 1 : x;
}

/*
 * Convert float to integer(1~255).
 */
inline uint8_t to_int(float x)
{
    return uint8_t (pow(clamp(x), 1 / 2.2) * 255 + .5);
}

#endif //GALLIFREY_COLORS_H
