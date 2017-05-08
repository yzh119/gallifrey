//
// Created by zhy96 on 4/2/17.
//

#ifndef GALLIFREY_TEXTURE_H
#define GALLIFREY_TEXTURE_H

#include "geometry.h"
#include <opencv2/opencv.hpp>

inline Vec get_pixel(cv::Mat *img, const Vec& pos)
{
    assert (img->data);
    float x = pos.x, y = pos.y;
    x -= floor(pos.x);
    y -= floor(pos.y);
    int j = (int) (x * img->rows), i = (int) (y * img->cols);
    return Vec((float) (img->at<cv::Vec3b>(j, i)[2] / 256.), (float) (img->at<cv::Vec3b>(j, i)[1] / 256.),
               (float) (img->at<cv::Vec3b>(j, i)[0] / 256.));
}

inline Vec get_texture_at_pos(const Triangle &t, Vec pos)
{
    Vec col;
    if (t.m->image == nullptr)
    {
        col = Vec(1, 1, 1);
    }
    else
    {
        float alpha, beta, gamma;
        locate(t, pos, alpha, beta, gamma);
        Vec t_pos = Vec(t.vt[0][0], t.vt[0][1], t.vt[0][2]) * alpha +
              Vec(t.vt[1][0], t.vt[1][1], t.vt[1][2]) * beta +
              Vec(t.vt[2][0], t.vt[2][1], t.vt[2][2]) * gamma;
        col = get_pixel(t.m->image, t_pos);
    }
    return col;
}

#endif //GALLIFREY_TEXTURE_H
