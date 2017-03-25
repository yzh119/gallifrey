//
// Created by zhy96 on 3/22/17.
//
#include "shader.h"
#include "io.h"\

extern const unsigned int width;
extern const unsigned int height;

int main(int argc, char *argv[])
{
    uint8_t col[3 * width * height];
    dump_bitmap((char *) "../out/dump.bmp", col);
    Face fArray[10000];
    Vertex vxArray[100000];
    Vector vnArray[10000];
    size_t la, lb, lc;
    obj_loader((char *) "../resources/cube.obj", fArray, vnArray, vxArray, la, lb, lc);
    printf("%d %d %d\n", (int) la, (int) lb, (int) lc);
    return 0;
}
