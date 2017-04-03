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
#define ADD_WALL(arg1, arg2, arg3, arg4) \
scene.f_array[scene.size_f].add_vx(idx - (arg1), -1, -1); \
scene.f_array[scene.size_f].add_vx(idx - (arg2), -1, -1); \
scene.f_array[scene.size_f].add_vx(idx - (arg3), -1, -1); \
scene.f_array[scene.size_f].add_vx(idx - (arg4), -1, -1); \
++scene.size_f
#define ADD_CORD(x, y, z) \
scene.vx_array[scene.size_vx++].set_coordinate((x), (y), (z))

extern const unsigned int width;
extern const unsigned int height;

const int max_face  = 120000;
const int max_vx    = 120000;
const int max_illu  = 300;
const int max_name  = 40;
const float view_dis = 1;

// Arguments
bool enable_anti_aliasing = false;
bool enable_shadow = false;
bool enable_global = false;
char model_name[max_name];
int num_workers = 4;


Face fArray[max_face];
Vec vxArray[max_vx];
Vec vnArray[max_vx];
Vec fnArray[max_face];
Vec ilArray[max_illu];
size_t l_face, l_vertex, l_normal;

Scene scene;
Image img(Vec(0, -5, -5), Vec(0, 1, 1), 2);

std::atomic<int> cnt_pixels;

inline float eta(int progress, int total,
        const std::chrono::high_resolution_clock::time_point &start,
        const std::chrono::high_resolution_clock::time_point &now)
{
    assert(progress != 0);
    return (float) ((1. * (total - progress) / progress) * ((now - start).count() / 1e9));
}

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
    min_x -= view_dis * len_x;
    min_y -= view_dis * len_y;
    min_z -= view_dis * len_z;

    max_x += view_dis * len_x;
    max_y += view_dis * len_y;
    max_z += view_dis * len_z;

    // Set the illumination;

    scene.il_array[scene.size_il++].set_coordinate(max_x - len_x / 2, max_y - len_y / 2,
                                                       max_z - len_z / 2);

    // Set multi-illumination to enable soft shadow.
    if (enable_shadow)
    {
        for (int i = -3; i < 3; ++i)
            for (int j = -3; j < 3; ++j)
                for (int k = -3; k < 3; ++k)
                {
                    if (i == 0 && j == 0 && k == 0) continue;
                    scene.il_array[scene.size_il++].set_coordinate((float) (max_x - len_x / 2 + 2e-1 * i),
                                                                   (float) (max_y - len_y / 2 + 2e-1 * j),
                                                                   (float) (max_z - len_z / 2 + 2e-1 * k));
                }
    }

    // Change the camera's view point.
    img.cam.d = Vec(-1, -1, 1).norm();
    img.cam.o.set_coordinate(max_x - len_x / 2, max_y - len_y / 2,
                             min_z + len_z / 2);

    img.adjust_camera();
    // Add the vertices to vertex list;

    ADD_CORD(min_x, min_y, min_z);
    ADD_CORD(min_x, min_y, max_z);
    ADD_CORD(min_x, max_y, min_z);
    ADD_CORD(min_x, max_y, max_z);
    ADD_CORD(max_x, min_y, min_z);
    ADD_CORD(max_x, min_y, max_z);
    ADD_CORD(max_x, max_y, min_z);
    ADD_CORD(max_x, max_y, max_z);

    int idx = (int) scene.size_vx;
    ADD_WALL(6, 5, 7, 8);
    ADD_WALL(4, 3, 1, 2);
    ADD_WALL(8, 7, 3, 4);
    ADD_WALL(2, 1, 5, 6);
    ADD_WALL(4, 2, 6, 8);
    ADD_WALL(7, 5, 1, 3);

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
    obj_loader((char *) ("../resources/" + std::string(model_name) + ".obj").c_str(), fArray, vnArray, vxArray, l_face, l_vertex, l_normal);
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
            if (enable_anti_aliasing)
            {
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
                            if (enable_global)
                                r = r + global_ill(Ray(img.cam.o, d.norm()), 0, scene) * (1. / img.samps);
                            else
                                r = r + local_ill(Ray(img.cam.o, d.norm()), scene) * (1. / img.samps);
                        }
                        col = col + r * .25;
                    }
            }
            else
            {
                Vec
                        d = img.cx * (1. * x / width  - .5) +
                            img.cy * (1. * y / height - .5) + img.cam.d;
                if (enable_global)
                    col = global_ill(Ray(img.cam.o, d.norm()), 0, scene);
                else
                    col = local_ill(Ray(img.cam.o, d.norm()), scene);
            }

            img.set_pixel(x, y, col);
            ++cnt_pixels;
            if (cnt_pixels.load() % 1024 == 0) {
                fprintf(stderr, "%*c\r", 79, ' ');
                fprintf(stderr, "Rendering the %d/%d pixel, ETA: %.2fs.\r",
                        cnt_pixels.load(), width * height,
                        eta(cnt_pixels.load(), height * width, start, std::chrono::high_resolution_clock::now()));
            }
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
            if ((y * width + x) % 1024 == 0 && y * width + x > 0) {
                fprintf(stderr, "%*c\r", 79, ' ');
                fprintf(stderr, "Rendering the %d/%d pixel, ETA: %.2fs.\r",
                        y * width + x, width * height,
                        eta(y * width + x, height * width, start, std::chrono::high_resolution_clock::now()));
            }
            if (enable_anti_aliasing){
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
                            if (enable_global)
                                r = r + global_ill(Ray(img.cam.o, d.norm()), 0, scene) * (1. / img.samps);
                            else
                                r = r + local_ill(Ray(img.cam.o, d.norm()), scene) * (1. / img.samps);
                        }
                        col = col + r * .25;
                    }
            }
            else
            {
                Vec
                        d = img.cx * (1. * x / width  - .5) +
                            img.cy * (1. * y / height - .5) + img.cam.d;
                if (enable_global)
                    col = global_ill(Ray(img.cam.o, d.norm()), 0, scene);
                else
                    col = local_ill(Ray(img.cam.o, d.norm()), scene);
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

void parse_argument(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--model") == 0)
        {
            strcpy(model_name, argv[++i]);
        } else if (strcmp(argv[i], "--anti_aliasing") == 0)
        {
            enable_anti_aliasing = true;
        } else if (strcmp(argv[i], "--shadow") == 0)
        {
            enable_shadow = true;
        } else if (strcmp(argv[i], "--global") == 0)
        {
            enable_global = true;
        } else if (strcmp(argv[i], "--core") == 0)
        {
            num_workers = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--help") == 0)
        {
            fprintf(stdout, "Gallifrey, a naive 3D engine.\n");
            fprintf(stdout, "--model MODEL_NAME:\t specifies the model name that the program loads\n");
            fprintf(stdout, "--core CORE:\t\t specifies the number of cores this program uses\n");
            fprintf(stdout, "--anti_aliasing:\t specifies whether to enable anti aliasing.\n");
            fprintf(stdout, "--shadow:\t\t specifies whether to enable (soft) shadow.\n");
            fprintf(stdout, "--global:\t\t specifies whether to enable global illumination.\n");
            fprintf(stdout, "--help:\t\t\t display this information.\n");
            fprintf(stdout, "Please refer to README.md for more details.\n");
        } else {
            fprintf(stderr, "Argument error, please use %s --help to see the usage.", argv[0]);
            exit(1);
        }
    }
    if (strlen(model_name) == 0)
    {
        fprintf(stderr, "Model name must be specified.");
        exit(1);
    }
    return ;
}

int main(int argc, char *argv[])
{
    // Initialization
    memset(model_name, '\0', sizeof(model_name));
#ifdef DEBUG
    enable_anti_aliasing = false;
    enable_shadow = true;
    enable_global = false;
    strcpy(model_name, "sphere");
#else
    parse_argument(argc, argv);
#endif

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
