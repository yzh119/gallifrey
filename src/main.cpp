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

size_t l_face, l_vertex, l_normal;

void load_module()
{
    obj_loader((char *) "../resources/roi.obj", fArray, vnArray, vxArray, l_face, l_vertex, l_normal);
    return ;
}

void create_image()
{
    return ;
}

void dump_image()
{
    return ;
}

int main(int argc, char *argv[])
{
    uint8_t col[3 * width * height];
    fill_pic_with_noise(col);
    dump_bitmap((char *) "../out/dump.bmp", col);
    size_t la, lb, lc = lb = la = 0;

    printf("%d %d %d\n", (int) la, (int) lb, (int) lc);
    test_intersection();
    /*
    for (int i = 0; i < la; i++) {
        printf("%d\n", fArray[i].get_size());
        for (int j = 0; j < fArray[i].get_size(); j++) {
            printf("%d ", fArray[i].get_elem_idxV(j));
        }
        printf("\n");
    }*/

    load_module();
    create_image();
    dump_image();
    return 0;
}
