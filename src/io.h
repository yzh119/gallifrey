//
// Created by zihao on 2017/3/23.
//

#ifndef GALLIFREY_IO_H
#define GALLIFREY_IO_H

#include "ply.h"
#include "bitmap_image.hpp"
#include <string>
#include <cstdlib>
#include <climits>

const unsigned int width = 640;
const unsigned int height = 480;

/*
 * Generate .bmp file from pixels.
 */
void dump_bitmap(const std::string &filename, uint8_t *col)
{
    bitmap_image dump(width, height);
    for (unsigned int y = 0; y < height; ++y)
        for (unsigned int x = 0; x < width; ++x)
        {
            int pos = y * width + x;
            rgb_t c;
            c.red   = col[3 * pos];
            c.green = col[3 * pos + 1];
            c.blue  = col[3 * pos + 2];
            dump.set_pixel(x, y, c);
        }
    dump.save_image(filename);
    return;
}

/*
 * Generate noise picture to test 'output_to_bmp'.
 */
void fill_pic_with_noise(uint8_t *col)
{
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            int pos = y * width + x;
            col[3 * pos]        = (uint8_t) (0xff * (1. * rand() / RAND_MAX));
            col[3 * pos + 1]    = (uint8_t) (0xff * (1. * rand() / RAND_MAX));
            col[3 * pos + 1]    = (uint8_t) (0xff * (1. * rand() / RAND_MAX));
        }
}

#endif //GALLIFREY_IO_H
