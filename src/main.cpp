//
// Created by zhy96 on 3/22/17.
//
#include "shader.h"
#include "io.h"

extern const unsigned int width;
extern const unsigned int height;

Face fArray[10000];
Vec vxArray[120000];
Vec vnArray[10000];
int main(int argc, char *argv[])
{
    uint8_t col[3 * width * height];
    fill_pic_with_noise(col);
    dump_bitmap((char *) "../out/dump.bmp", col);
    size_t la, lb, lc = lb = la = 0;

    obj_loader((char *) "../resources/roi.obj", fArray, vnArray, vxArray, la, lb, lc);
    printf("%d %d %d\n", (int) la, (int) lb, (int) lc);
    /*
    for (int i = 0; i < la; i++) {
        printf("%d\n", fArray[i].get_size());
        for (int j = 0; j < fArray[i].get_size(); j++) {
            printf("%d ", fArray[i].get_elem_idxF(j));
        }
        printf("\n");
    }*/

    return 0;
}
