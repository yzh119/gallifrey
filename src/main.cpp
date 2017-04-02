//
// Created by zhy96 on 3/22/17.
//
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
Vec ilArray[max_illu];

size_t l_face, l_vertex, l_normal;

Scene scene;

Image img(Vec(0, -5, -5), Vec(0, 1, 1), 1);

std::atomic<int> cnt_pixels;
const int num_workers = 4;

inline float erand()
{
    return (float) rand() / RAND_MAX;
}

/*
 * To create the bound box that covers the entire scene.
 */
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
    min_x -= 2 * len_x;
    min_y -= 2 * len_y;
    min_z -= 2 * len_z;

    max_x += 2 * len_x;
    max_y += 2 * len_y;
    max_z += 2 * len_z;

    // Set the illumination;

    scene.il_array[scene.size_il++].set_coordinate(min_x + len_x / 2, min_y + len_y / 2,
                                                       max_z - len_z / 2);

    // Change the camera's view point.
    img.cam.d = Vec(-1, 1, -1).norm();
    img.cam.o.set_coordinate(max_x - len_x / 2, min_y + len_y / 2,
                             max_z - len_z / 2);

    img.adjust_camera();
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
    scene.f_array[scene.size_f].add_vx(idx - 6, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 5, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 7, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 8, -1, -1);
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

    scene.f_array[scene.size_f].add_vx(idx - 2, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 1, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 5, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 6, -1, -1);
    ++scene.size_f;

    scene.f_array[scene.size_f].add_vx(idx - 4, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 2, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 6, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 8, -1, -1);
    ++scene.size_f;

    scene.f_array[scene.size_f].add_vx(idx - 7, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 5, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 1, -1, -1);
    scene.f_array[scene.size_f].add_vx(idx - 3, -1, -1);
    ++scene.size_f;

    // Set the 'ka' parameter manually.
    for (int i = (int) (scene.size_f - 6); i < scene.size_f; ++i)
    {
        scene.f_array[i].set_ka(Vec(.5, .5, .5));
    }
    return ;
}

void load_and_construct_scene()
{
    auto start = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "Loading... \n");
    obj_loader((char *) "../resources/teapot.obj", fArray, vnArray, vxArray, l_face, l_vertex, l_normal);
    scene.f_array   = fArray;
    scene.vn_array  = vnArray;
    scene.vx_array  = vxArray;
    scene.fn_array  = fnArray;
    scene.il_array  = ilArray;
    scene.size_f    = l_face;
    scene.size_vn   = l_normal;
    scene.size_vx   = l_vertex;
    scene.size_il = 0;

    add_wall_illumination();

    // Compute the normal vector.
    for (int i = 0; i < scene.size_f; ++i)
    {
        Vec &v = scene.fn_array[i];
        for (int j = 0; j < scene.f_array[i].get_size(); ++j)
            v = v + (scene.vx_array[scene.f_array[i].get_elem_idxV(j)] %
                    scene.vx_array[scene.f_array[i].get_elem_idxV(j + 1)]);
        v.norm();
    }

    // Compute the vertex normal vector.
    for (int i = 0; i < scene.size_f; ++i)
    {
        for (int j = 0; j < scene.f_array[i].get_size(); ++j)
        {
            size_t idx = scene.f_array[i].get_elem_idxV(j) + scene.size_vn;
            Vec &v = scene.vn_array[idx];
            v = v + scene.fn_array[i];
            if (scene.f_array[i].get_elem_idxVn(j) == -1)
            {
                scene.f_array[i].modify_vn(j, (int) idx);
            }
        }
    }

    // Normalization
    for (int i = (int) scene.size_vn; i < scene.size_vn + scene.size_vx; ++i){
        scene.vn_array[i].norm();
    }

    scene.size_vn += scene.size_vx;


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
                        r = r + radiance(Ray(img.cam.o, d.norm()), 0, scene) * (1. / img.samps);
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
            if ((y * width + x) % 1024 == 0) fprintf(stderr, "Rendering the %d/%d pixel.\r", y * width + x, width * height);
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

                        r = r + radiance(Ray(img.cam.o, d.norm()), 0, scene) * (1. / img.samps);
                    }
                    col = col + r * .25;
                }
            img.set_pixel(x, y, col);
        }
    }
    fprintf(stderr, "\n");
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
