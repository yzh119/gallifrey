//
// Created by zhy96 on 5/22/17.
//

#include "reconstr.h"
#include "bitmap_image.hpp"

extern const unsigned int width, height;

float dump_samps[32 * width * height * 3];
float aver_samps[width * height * 3];
float variance[width * height * 3];
float parameters[width * height * 7];
float reconstr[width * height * 3];
uint8_t gen_image[width * height * 3];

/*
 * Color amplification function. (Let upper bound be 1 and lower bound be 0)
 */
inline float clamp(float x)
{
    return x < 0 ? 0 : x > 1 ? 1 : x;
}

/*
 * Convert float to integer(range from 0 to 255).
 */
inline uint8_t to_int(float x)
{
    return uint8_t (pow(clamp(x), 1 / 2.2) * 255 + .5);
}

inline void dump_bitmap(char *path, uint8_t *col)
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
    dump.save_image(path);
    return;
}

void load_parameters()
{
    FILE *pDumpPara;
    pDumpPara = fopen("../../out/params.bin", "rb");
    fread(parameters, sizeof(float), sizeof(parameters) / sizeof(float), pDumpPara);
    fclose(pDumpPara);
    return;
}

void load_samples()
{
    FILE *pDumpSamp;
    pDumpSamp = fopen("../../out/samps.bin", "rb");
    fread(dump_samps, sizeof(float), sizeof(dump_samps) / sizeof(float), pDumpSamp);
    fclose(pDumpSamp);
    for (int s = 0; s < 32; ++s) {
        for (unsigned int y = 0; y < height; ++y)
            for (unsigned int x = 0; x < width; ++x) {
                int pos = y * width + x;
                aver_samps[pos * 3 + 0] += dump_samps[(s * width * height + pos) * 3 + 0] / 32;
                aver_samps[pos * 3 + 1] += dump_samps[(s * width * height + pos) * 3 + 1] / 32;
                aver_samps[pos * 3 + 2] += dump_samps[(s * width * height + pos) * 3 + 2] / 32;
            }
    }
    for (int s = 0; s < 32; ++s) {
        for (unsigned int y = 0; y < height; ++y)
            for (unsigned int x = 0; x < width; ++x) {
                int pos = y * width + x;
                variance[pos * 3 + 0] += (dump_samps[(s * width * height + pos) * 3 + 0] - aver_samps[pos * 3 + 0]) *
                                         (dump_samps[(s * width * height + pos) * 3 + 0] - aver_samps[pos * 3 + 0]);
                variance[pos * 3 + 1] += (dump_samps[(s * width * height + pos) * 3 + 1] - aver_samps[pos * 3 + 1]) *
                                         (dump_samps[(s * width * height + pos) * 3 + 1] - aver_samps[pos * 3 + 1]);
                variance[pos * 3 + 2] += (dump_samps[(s * width * height + pos) * 3 + 2] - aver_samps[pos * 3 + 2]) *
                                         (dump_samps[(s * width * height + pos) * 3 + 2] - aver_samps[pos * 3 + 2]);
            }
    }
    return;
}

void dump_reconstructed_bmp()
{
    for (unsigned int y = 0; y < height; ++y)
        for (unsigned int x = 0; x < width; ++x)
        {
            int pos = (height - 1 - y) * width + x;
            gen_image[3 * pos]     = to_int(reconstr[pos * 3]);
            gen_image[3 * pos + 1] = to_int(reconstr[pos * 3 + 1]);
            gen_image[3 * pos + 2] = to_int(reconstr[pos * 3 + 2]);
        }
    dump_bitmap((char *) "../../out/recons1.bmp", gen_image);
}

int main(int argc, char *argv[])
{
    load_parameters();
    load_samples();
    outlier_removal();
   // for (int stage = 0; stage < 5; ++stage) {
        compute_coefficient(parameters, aver_samps, reconstr);
   // }

    dump_reconstructed_bmp();
    fprintf(stderr, "DONE\n");
    return 0;
}