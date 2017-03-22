//
// Created by zhy96 on 3/22/17.
//
#include "shader.h"
#include "io.h"

extern const unsigned int width;
extern const unsigned int height;

int main(int argc, char *argv[])
{
    uint8_t col[3 * width * height];
    fill_pic_with_noise(col);
    dump_bitmap("../out/dump.bmp", col);
    return 0;
}
