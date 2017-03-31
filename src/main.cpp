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
#include <atomic>

extern const unsigned int width;
extern const unsigned int height;

const int max_face  = 120000;
const int max_vx    = 120000;
const int max_illu  = 20;

Face fArray[max_face];
Vec vxArray[max_vx];
Vec vnArray[max_vx];
Vec fnArray[max_face];
Vec illuArray[max_illu];

size_t l_face, l_vertex, l_normal;

Scene scene;

Image img(Vec(0, -5, -5), Vec(0, 1, 1));

std::atomic<int> cnt_pixels;
const int num_workers = 4;

inline float erand()
{
    return (float) rand() / RAND_MAX;
}

inline void add_wall_illumination()
{
    float min_x, min_y, min_z = min_y = min_x = 1e9;
    float max_x, max_y, max_z = max_y = max_x = (float) -1e9;
    for (int i = 0; i < scene.size_vx; ++i)
    {
        if (scene.vx_array[i].x < min_x) min_x = scene.vx_array[i].x;
        if (scene.vx_array[i].y < min_y) min_y = scene.vx_array[i].y;
        if (scene.vx_array[i].z < min_z) min_z = scene.vx_array[i].z;

        if (scene.vx_array[i].x > max_x) max_x = scene.vx_array[i].x;
        if (scene.vx_array[i].y > max_y) max_y = scene.vx_array[i].y;
        if (scene.vx_array[i].z > max_z) max_z = scene.vx_array[i].z;
    }

    float
        len_x = max_x - min_x,
        len_y = max_y - min_y,
        len_z = max_z - min_z;

    // Calculate size of the box;
    min_x -= 3 * len_x;
    min_y -= 3 * len_y;
    min_z -= 3 * len_z;

    max_x += 3 * len_x;
    max_y += 3 * len_y;
    max_z += 3 * len_z;

    printf("%f %f %f %f %f %f", min_x, min_y, min_z, max_x, max_y, max_z);

    // Set the illumination;

    scene.illu_array[scene.size_illu++].set_coordinate((float) (min_x + .5 * len_x), (float) (min_y + .5 * len_y),
                                                       (float) (max_z - .5 * len_z));

    // Change the camera's view point.
    img.cam.d.set_coordinate(-.3, .3, -.3);
    // img.cam.o.set_coordinate((float) (max_x - .5 * len_x), (float) (min_y + .5 * len_y),
    //                         (float) (max_z - .5 * len_z));
    img.cam.o.set_coordinate((float) max_x, (float) min_y,
                             (float) max_z);

    // Add the vertices to vertex list;

    scene.vx_array[scene.size_vx++].set_coordinate(min_x, min_y, min_z);
    scene.vx_array[scene.size_vx++].set_coordinate(min_x, min_y, max_z);
    scene.vx_array[scene.size_vx++].set_coordinate(min_x, max_y, min_z);
    scene.vx_array[scene.size_vx++].set_coordinate(min_x, max_y, max_z);
    scene.vx_array[scene.size_vx++].set_coordinate(max_x, min_y, min_z);
    scene.vx_array[scene.size_vx++].set_coordinate(max_x, min_y, max_z);
    scene.vx_array[scene.size_vx++].set_coordinate(max_x, max_y, min_z);
    scene.vx_array[scene.size_vx++].set_coordinate(max_x, max_y, max_z);

    int idx = (int) scene.size_vx;
    scene.f_array[scene.size_f].add_vx(idx - 8, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 7, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 5, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 6, -1, -1);
    ++scene.size_f;

    scene.f_array[scene.size_f].add_vx(idx - 4, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 3, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 1, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 2, -1, -1);
    ++scene.size_f;

    scene.f_array[scene.size_f].add_vx(idx - 8, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 7, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 3, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 4, -1, -1);
    ++scene.size_f;

    scene.f_array[scene.size_f].add_vx(idx - 6, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 5, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 1, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 2, -1, -1);
    ++scene.size_f;

    scene.f_array[scene.size_f].add_vx(idx - 8, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 6, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 2, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 4, -1, -1);
    ++scene.size_f;

    scene.f_array[scene.size_f].add_vx(idx - 7, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 5, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 1, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 3, -1, -1);
    ++scene.size_f;

    for (int i = scene.size_f - 6; i < scene.size_f; ++i)
    {
        scene.f_array[i].set_ka(Vec(.3, .2, .4));
    }

    return ;
}

void load_and_construct_scene()
{
    auto start = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "Loading... \n");
    obj_loader((char *) "../resources/cube.obj", fArray, vnArray, vxArray, l_face, l_vertex, l_normal);
    scene.f_array   = fArray;
    scene.vn_array  = vnArray;
    scene.vx_array  = vxArray;
    scene.fn_array  = fnArray;
    scene.illu_array = illuArray;
    scene.size_f    = l_face;
    scene.size_vn   = l_normal;
    scene.size_vx   = l_vertex;
    scene.size_illu = 0;
    for (int i = 0; i < l_face; ++i)
    {
        Vec v1 = scene.f_array->get_elem_idxV(1) - scene.f_array->get_elem_idxV(0),
            v2 = scene.f_array->get_elem_idxV(2) - scene.f_array->get_elem_idxV(1);
        scene.fn_array[i] = (v1 % v2).norm();
    }

    add_wall_illumination();

    auto now = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "Load successful in %.3fs.\n", (now - start).count() / 1e9);
    return ;
}

void rendering()
{
    auto start = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "Rendering... ");

    moodycamel::ConcurrentQueue<std::pair<int, int> > q;

    // Thanks Lequn Chen(abcdabcd987) for his guidance on multi-threading programming.

#ifdef __linux__
    auto func = [&]
    {
        for (std::pair<int, int> item;;)
        {
            q.try_dequeue(item);
            int x = item.first,
                y = item.second;
            if (x == -1) return;
            Vec col(0, 0, 0);
            for (int sy = 0; sy < 2; ++sy)
                for (int sx = 0; sx < 2; ++sx) {
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
            if (cnt_pixels.load() % 1024 == 0)
                fprintf(stderr, "Rendering the %d/%d pixel.\r", cnt_pixels.load(), (width * height));
            ++cnt_pixels;
        }
    };

    cnt_pixels = 0;
    std::vector <std::pair<int, int> > xys;
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            xys.emplace_back(x, y);
        }
    std::random_shuffle(xys.begin(), xys.end());

    for (const auto &xy: xys)
        q.enqueue(xy);

    std::vector<std::thread> workers;

    for (int i = 0; i < num_workers; ++i)
        workers.emplace_back(func);
    for (int i = 0; i < num_workers; ++i)
        q.enqueue(std::make_pair(-1, -1));

    while (true)
    {
        int cnt = cnt_pixels.load();
        if (cnt == width * height) {
            fprintf(stderr, "\n");
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    for (auto &worker: workers)
        worker.join();
#endif
#ifdef _WIN32
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
                        printf("%f %f %f\n", 140 * d.x, 140 * d.y, 140 * d.z);
                        r = r + radiance(Ray(img.cam.o + d * 140, d.norm()), 0, scene) * (1. / img.samps);
                    }
                    col = col + r * .25;
                }
            img.set_pixel(x, y, col);
        }
    }
#endif

    auto now = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "Render successful in %.3fs.\n", (now - start).count() / 1e9);
    return ;
}

void dump_image()
{
    auto start = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "Generating... \n");
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
    fprintf(stderr, "DONE.\n");
    return 0;
}
