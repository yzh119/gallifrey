//
// Created by zhy96 on 4/2/17.
//

#ifndef GALLIFREY_TEXTURE_H
#define GALLIFREY_TEXTURE_H

#include "geometry.h"
#include <opencv2/opencv.hpp>

inline Vec get_pixel(cv::Mat *img, float x, float y)
{
    assert (img->data);
    x -= floor(x);
    y -= floor(y);
    int j = (int) (x * img->rows), i = (int) (y * img->cols);
    return Vec((float) (img->at<cv::Vec3b>(j, i)[2] / 256.), (float) (img->at<cv::Vec3b>(j, i)[1] / 256.),
               (float) (img->at<cv::Vec3b>(j, i)[0] / 256.));
}

inline Vec get_texture_at_pos(const Triangle &t, const Vec &pos)
{
    if (t.m->image == nullptr)
    {
        return Vec(1, 1, 1);
    }
    else
    {
        float alpha, beta, gamma;
        locate(t, pos, alpha, beta, gamma);
        return get_pixel(t.m->image, t.vt[0][0] * alpha + t.vt[1][0] * beta + t.vt[2][0] * gamma, t.vt[0][1] * alpha + t.vt[1][1] * beta + t.vt[2][1] * gamma);
    }
}

#endif //GALLIFREY_TEXTURE_H
