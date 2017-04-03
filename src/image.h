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
    float tan;
public:
    Image(const Vec &pos, const Vec &dir, float tan): cam(Ray(pos, dir)), tan(tan)
    {
        cy = (Vec(cam.d.y, -cam.d.x, 0).norm() * tan);
        cx = (cy % cam.d).norm() * (tan * width / height);
        cy = Vec(-cy.x, -cy.y, -cy.z);
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
    inline void adjust_camera();
};

uint8_t *Image::to_bmp_pixel()
{
    for (unsigned int y = 0; y < height; ++y)
        for (unsigned int x = 0; x < width; ++x)
        {
            int pos = (height - 1 - y) * width + x;
            int pos1 = y * width + x;
            output[3 * pos1]     = to_int(col[3 * pos1]);
            output[3 * pos1 + 1] = to_int(col[3 * pos1 + 1]);
            output[3 * pos1 + 2] = to_int(col[3 * pos1 + 2]);
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

inline void Image::adjust_camera() {
    cy = (Vec(cam.d.y, 2 * -cam.d.x).norm() * tan);
    cx = (cy % cam.d).norm() * (tan * width / height);
    cy = Vec(-cy.x, -cy.y, -cy.z);
}

#endif //GALLIFREY_IMAGE_H
