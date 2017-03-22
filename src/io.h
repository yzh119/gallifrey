//
// Created by zihao on 2017/3/23.
//

#ifndef GALLIFREY_IO_H
#define GALLIFREY_IO_H

#include "ply.h"
#include <string>
#include <cstdlib>
#include <climits>
#include "

const unsigned int width = 640;
const unsigned int height = 480;

/*
 * Generate .ppm file from pixels.
 */
void output_ppm(const std::string &filename, uint8_t *col)
{
    FILE *f = fopen(filename.c_str(), "w");
    fprintf(f, "P3\n");
    fprintf(f, "%d %d\n", width, height);
    fprintf(f, "255\n");
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int pos = y * width + x;
            fprintf(f, "%u\t%u\t%u\t", col[3 * pos], col[3 * pos + 1], col[3 * pos + 2]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
    return;
}

/*
 * Generate noise picture to test 'output_to_ppm'.
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
