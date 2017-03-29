//
// Created by zhy96 on 3/22/17.
//
#include "shader.h"
#include "io.h"
#include "image.h"
#include "tracing.h"

extern const unsigned int width;
extern const unsigned int height;

Face fArray[10000];
Vec vxArray[120000];
Vec vnArray[10000];

size_t l_face, l_vertex, l_normal;

inline float erand()
{
    return (float) rand() / RAND_MAX;
}

void load_module()
{
    obj_loader((char *) "../resources/roi.obj", fArray, vnArray, vxArray, l_face, l_vertex, l_normal);
    printf("%d %d %d", l_face, l_vertex, l_normal);
    return ;
}

void create_image()
{
    Image img(Vec(0, 0, 0), Vec(0, 0, 1));
    const int samps = 1;
    for (unsigned int y = 0; y < height; ++y)
        for (unsigned int x = 0; x < width; ++x)
        {
            for (unsigned int sy = 0; sy < 2; ++sy)
                for (unsigned int sx = 0; sx < 2; ++sx)
                {
                    Vec r();
                    for (int s = 0; s < samps; ++s)
                    {
                        float
                                r1 = 2 * erand(),
                                r2 = 2 * erand();
                        float
                                dx = (float) (r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1)),
                                dy = (float) (r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2));
                        //Vec d = img.cx + img.cy;
                        //r = r + radiance();
                    }
                }
        }
    return ;
}

void dump_image()
{
    uint8_t col[3 * width * height];
    fill_pic_with_noise(col);
    dump_bitmap((char *) "../out/dump.bmp", col);
    return ;
}

int main(int argc, char *argv[])
{
    // Test procedure.
    test_intersection();

    // Main procedure.
    load_module();
    create_image();
    dump_image();
    return 0;
}
