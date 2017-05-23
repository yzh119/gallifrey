//
// Created by zhy96 on 5/8/17.
//

#ifndef GALLIFREY_RECONSTRUCTION_H
#define GALLIFREY_RECONSTRUCTION_H
#include <armadillo>
#include <cmath>
#include <algorithm>

const int half_window = 7;
const int step_size = 7;

const unsigned int
        width = 1280,
        height = 960;

float weight[width * height];

inline float gaussian_kernel(float dx, float dy)
{
    return expf(-(dx * dx + dy * dy) / (half_window * half_window / 2));
}

void outlier_removal()
{
    return;
}

void select_order(unsigned int *order, float *mean, float *variance)
{
    for (unsigned int y = 0; y < height; ++y)
        for (unsigned int x = 0; x < width; ++x)
        {
            order[y * width + x] = 1;
        }
}

void compute_coefficient(float *params, float *samps, unsigned int *order, float *ret)
{
    std::fill(weight, weight + width * height, 0);
    for (unsigned int cy = step_size; cy < height; cy += step_size)
        for (unsigned int cx = step_size; cx < width; cx += step_size) {
            std::vector<arma::Row<float> > Xrows;
            std::vector<float> ws;
            std::vector<float> ysr, ysg, ysb;
            int cnt = 0, ord = order[cy * width + cx];
            for (int dx = -half_window; dx <= half_window; ++dx)
                for (int dy = -half_window; dy <= half_window; ++dy) {
                    int x = cx + dx, y = cy + dy;
                    int pos = y * width + x;
                    if (x < 0 || x >= width) continue;
                    if (y < 0 || y >= height) continue;
                    arma::Row<float> rowvec((const arma::uword) (2 * ord + 8));
                    rowvec(0) = 1;
                    for (int j = 0; j < ord; ++j)
                    {
                        rowvec((const arma::uword) (2 * j + 1)) = powf(float(dx) / float(width), float(j + 1));
                        rowvec((const arma::uword) (2 * j + 2)) = powf(float(dy) / float(height), float(j + 1));
                    }
                    for (int j = 2 * ord + 1; j < 2 * ord + 8; ++j)
                        rowvec(j) = params[7 * pos + j - 3];
                    Xrows.push_back(rowvec);
                    ws.push_back(gaussian_kernel((float) dx, (float) dy));
                    weight[pos] += ws.back();
                    ysr.push_back(samps[3 * pos + 0]);
                    ysg.push_back(samps[3 * pos + 1]);
                    ysb.push_back(samps[3 * pos + 2]);
                    ++cnt;
                }
            arma::Mat<float> X((const arma::uword) cnt, (const arma::uword) (2 * ord + 8));
            arma::Col<float>
                    YR((const arma::uword) cnt),
                    YG((const arma::uword) cnt),
                    YB((const arma::uword) cnt);
            arma::Mat<float> W((const arma::uword) cnt, (const arma::uword) cnt);//(arma::diagmat(ws));
            for (int i = 0; i < cnt; ++i) {
                X.row(i) = Xrows[i];
                YR(i) = ysr[i];
                YG(i) = ysg[i];
                YB(i) = ysb[i];
                W(i, i) = ws[i];
            }

            arma::Mat<float> H = X * arma::pinv(X.t() * W * X) * X.t() * W;
            arma::Mat<float>
                    rec_yr = H * YR,
                    rec_yg = H * YG,
                    rec_yb = H * YB;
            cnt = 0;

            for (int dx = -half_window; dx <= half_window; ++dx)
                for (int dy = -half_window; dy <= half_window; ++dy) {
                    int x = cx + dx, y = cy + dy;
                    int pos = y * width + x;
                    if (x < 0 || x >= width) continue;
                    if (y < 0 || y >= height) continue;
                    ret[3 * pos] += ws[cnt] * rec_yr(cnt);
                    ret[3 * pos + 1] += ws[cnt] * rec_yg(cnt);
                    ret[3 * pos + 2] += ws[cnt] * rec_yb(cnt);
                    ++cnt;
                }
        }

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            int pos = y * width + x;
            ret[3 * pos] /= weight[pos];
            ret[3 * pos + 1] /= weight[pos];
            ret[3 * pos + 2] /= weight[pos];
        }
}

#endif //GALLIFREY_RECONSTRUCTION_H
