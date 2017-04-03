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
    //printf("%d %d\n", img->rows, img->cols);
    float x = pos.x, y = pos.y;
    x -= floor(pos.x);
    y -= floor(pos.y);
    int j = (int) (x * img->rows), i = (int) (y * img->cols);
    return Vec((float) (img->at<cv::Vec3b>(j, i)[2] / 256.), (float) (img->at<cv::Vec3b>(j, i)[1] / 256.),
               (float) (img->at<cv::Vec3b>(j, i)[0] / 256.));
}

inline Vec get_texture_at_pos(const Face &f, Vec pos, const Scene &s)
{
    Vec col;
    if (f.material.image == nullptr)
    {
        col = Vec(1, 1, 1);
    }
    else
    {
        int j;
        float alpha, beta, gamma;
        locate(f, pos, s, j, alpha, beta, gamma);
        Vec t_pos = s.vt_array[f.get_elem_idxVt(0)] * alpha +
              s.vt_array[f.get_elem_idxVt(j)] * beta +
              s.vt_array[f.get_elem_idxVt(j + 1)] * gamma;
        col = get_pixel(f.material.image, t_pos);
    }
    return col;
}

#endif //GALLIFREY_TEXTURE_H
