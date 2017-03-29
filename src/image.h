//
// Created by zihao on 2017/3/26.
//

#ifndef GALLIFREY_IMAGE_H
#define GALLIFREY_IMAGE_H

#include "geometry.h"
#include "colors.h"

extern const unsigned int width;
extern const unsigned int height;

class Image
{
private:
    // position and dir of camera.
    // x/y direction increment.
    float *col;                 // col array with type float.
    float r[3];                 // colors of samples.
    uint8_t *output;            // col array with type uint8_t for conversion.
public:
    Image(const Vec &pos, const Vec &dir): cam(Ray(pos, dir)), cx(Vec((float) (.5135 * width / height), 0, 0))
    {
        cy = (cx % cam.d).norm() * .5135;
        samps = 1;
        col     = new float[3 * width * height];
        output  = new uint8_t[3 * width * height];
    }

    inline void set_pixel(int x, int y, const Vec &rad);    // Set pixel at coordinate (x, y)
    inline uint8_t *to_bmp_pixel();                         // Convert float pixel (range from 0 to 1) to int8 pixel(range from 0 to 255).
    Vec cx;
    Vec cy;
    Ray cam;
    int samps;                                              // number of samples.
};

uint8_t *Image::to_bmp_pixel()
{
    for (unsigned int y = 0; y < height; ++y)
        for (unsigned int x = 0; x < width; ++x)
        {
            int pos = y * width + x;
            output[3 * pos]     = to_int(col[3 * pos]);
            output[3 * pos + 1] = to_int(col[3 * pos + 1]);
            output[3 * pos + 2] = to_int(col[3 * pos + 2]);
        }
    return output;
}

void Image::set_pixel(int x, int y, const Vec &rad)
{
    int pos = y * width + x;
    this->col[3 * pos] = rad.x;
    this->col[3 * pos + 1] = rad.y;
    this->col[3 * pos + 2] = rad.z;
}

#endif //GALLIFREY_IMAGE_H
