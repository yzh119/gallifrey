//
// Created by zhy96 on 3/22/17.
//
#include "shader.h"
#include "io.h"
#include "image.h"
#include "tracing.h"
#include "concurrentqueue.h"
#include <thread>
#include <chrono>

extern const unsigned int width;
extern const unsigned int height;

Face fArray[120000];
Vec vxArray[120000];
Vec vnArray[120000];

size_t l_face, l_vertex, l_normal;

Scene scene;

Image img(Vec(0, -5, -5), Vec(0, 1, 1));

inline float erand()
{
    return (float) rand() / RAND_MAX;
}

void load_and_construct_scene()
{
    auto start = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "Loading... ");
    obj_loader((char *) "../resources/sphere.obj", fArray, vnArray, vxArray, l_face, l_vertex, l_normal);
    scene.f_array = fArray;
    scene.vn_array = vnArray;
    scene.vx_array = vxArray;
    scene.size_f = l_face;
    scene.size_vn = l_normal;
    scene.size_vx = l_vertex;
    auto now = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "Load successful in %.3fs.\n", (now - start).count() / 1e9);
    return ;
}

void rendering()
{
    auto start = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "Rendering... ");
    // moodycamel::ConcurrentQueue<std::pair<int, int> > q;

    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            Vec col(0, 0, 0);
            for (unsigned int sy = 0; sy < 2; ++sy)
                for (unsigned int sx = 0; sx < 2; ++sx) {
                    Vec r(0, 0, 0);
                    for (int s = 0; s < img.samps; ++s) {
                        float
                                r1 = 2 * erand(),
                                r2 = 2 * erand();
                        float
                                dx = (float) (r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1)),
                                dy = (float) (r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2));
                        Vec
                                d = img.cx * (((sx + .5 + dx) / 2 + x) / width  - .5) +
                                    img.cy * (((sy + .5 + dy) / 2 + y) / height - .5) + img.cam.d;
                        r = r + radiance(Ray(img.cam.o + d * 5, d.norm()), 0, scene) * (1. / img.samps);
                    }
                    col = col + r * .25;
                }
            img.set_pixel(x, y, col);
        }
    }
    auto now = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "Render successful in %.3fs.\n", (now - start).count() / 1e9);
    return ;
}

void dump_image()
{
    auto start = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "Generating... ");
    dump_bitmap((char *) "../out/dump.bmp", img.to_bmp_pixel());
    auto now = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "Generate successful in %.3fs.\n", (now - start).count() / 1e9);
    return ;
}

int main(int argc, char *argv[])
{
    // unit test.
    test_dump_image();
    test_intersection();

    // main.
    load_and_construct_scene();
    rendering();
    dump_image();
    return 0;
}
