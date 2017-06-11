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
const int max_ord = 4;

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

void compute_coefficient(float *params, float *samps, float *ret)
{
    std::fill(weight, weight + width * height, 0);

    for (unsigned int cy = step_size; cy < height; cy += step_size)
        for (unsigned int cx = step_size; cx < width; cx += step_size) {
            arma::Mat<float> best_yr, best_yg, best_yb;
            float loss = 1e15;

            std::vector<float> ws;
            for (int dx = -half_window; dx <= half_window; ++dx)
                for (int dy = -half_window; dy <= half_window; ++dy) {
                    int x = cx + dx, y = cy + dy;
                    if (x < 0 || x >= width) continue;
                    if (y < 0 || y >= height) continue;
                    ws.push_back(gaussian_kernel((float) dx, (float) dy));
                }

            for (unsigned int ord = 0; ord < max_ord; ++ord) {
                std::vector<arma::Row<float> > Xrows;
                std::vector<float> ysr, ysg, ysb;
                int cnt = 0;
                for (int dx = -half_window; dx <= half_window; ++dx)
                    for (int dy = -half_window; dy <= half_window; ++dy) {
                        int x = cx + dx, y = cy + dy;
                        int pos = y * width + x;
                        if (x < 0 || x >= width) continue;
                        if (y < 0 || y >= height) continue;
                        arma::Row<float> rowvec((const arma::uword) (2 * ord + 8));
                        rowvec(0) = 1;
                        for (int j = 0; j < ord; ++j) {
                            rowvec((const arma::uword) (2 * j + 1)) = powf(float(dx) / float(width), float(j + 1));
                            rowvec((const arma::uword) (2 * j + 2)) = powf(float(dy) / float(height), float(j + 1));
                        }
                        for (int j = 2 * ord + 1; j < 2 * ord + 8; ++j)
                            rowvec(j) = params[7 * pos + j - 3];
                        Xrows.push_back(rowvec);
                        weight[pos] += ws[cnt];
                        ysr.push_back(samps[3 * pos + 0]);
                        ysg.push_back(samps[3 * pos + 1]);
                        ysb.push_back(samps[3 * pos + 2]);
                        ++cnt;
                    }
                arma::Mat<float> X((const arma::uword) cnt, (const arma::uword) (2 * ord + 8));
                arma::Col<float> YR(ysr), YG(ysg), YB(ysb);
                arma::Mat<float> W(arma::diagmat(arma::Row<float>(ws)));
                for (int i = 0; i < cnt; ++i)
                    X.row(i) = Xrows[i];

                arma::Mat<float> H = X * arma::pinv(X.t() * W * X) * X.t() * W;
                arma::Mat<float>
                        rec_yr = H * YR,
                        rec_yg = H * YG,
                        rec_yb = H * YB;

                float current_loss = arma::norm(W * (rec_yr - YR)) + arma::norm(W * (rec_yg - YG)) + arma::norm(W * (rec_yb - YB));
                if (current_loss < loss) {
                    best_yr = rec_yr;
                    best_yg = rec_yg;
                    best_yb = rec_yb;
                }
            }

            int cnt = 0;

            for (int dx = -half_window; dx <= half_window; ++dx)
                for (int dy = -half_window; dy <= half_window; ++dy) {
                    int x = cx + dx, y = cy + dy;
                    int pos = y * width + x;
                    if (x < 0 || x >= width) continue;
                    if (y < 0 || y >= height) continue;
                    ret[3 * pos] += max_ord * ws[cnt] * best_yr(cnt);
                    ret[3 * pos + 1] += max_ord * ws[cnt] * best_yg(cnt);
                    ret[3 * pos + 2] += max_ord * ws[cnt] * best_yb(cnt);
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
