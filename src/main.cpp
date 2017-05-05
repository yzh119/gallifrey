//
// Created by zhy96 on 3/22/17.
//
#include "io.h"
#include "image.h"
#include "tracing.h"
#include "concurrentqueue.h"
#include "json.hpp"
#include "kdtree.h"
#include <thread>
#include <chrono>
#include <atomic>
#define ADD_WALL(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) \
scene.f_array[scene.size_f].material = (arg9);\
scene.f_array[scene.size_f].add_vx(idx - (arg1), -1, idt - (arg5)); \
scene.f_array[scene.size_f].add_vx(idx - (arg2), -1, idt - (arg6)); \
scene.f_array[scene.size_f].add_vx(idx - (arg3), -1, idt - (arg7)); \
scene.f_array[scene.size_f].add_vx(idx - (arg4), -1, idt - (arg8)); \
++scene.size_f
#define ADD_CORD(x, y, z) \
scene.vx_array[scene.size_vx++].set_coordinate((x), (y), (z))
#define ADD_ILLU(color, X, Y, Z) \
scene.li_array[scene.size_il].set_coordinate(color); \
scene.il_array[scene.size_il++].set_coordinate((X), (Y), (Z))
#define ADD_TEX_CORD(x, y) \
scene.vt_array[scene.size_vt++].set_coordinate((x), (y), 0)

extern const unsigned int width;
extern const unsigned int height;

const int max_face  = 1000000;
const int max_vx    = 1000000;
const int max_illu  = 300;
const int max_name  = 40;

// Arguments
bool enable_anti_aliasing = false;
bool enable_shadow  = false;
bool enable_global  = false;
bool enable_display = false;
bool enable_sah     = false;
float view_dis = 1.5;
char config_name[max_name];
int num_workers = 4;
int num_samples = 2;

cv::Mat wall_mat = cv::imread("../resources/wall.jpg", CV_LOAD_IMAGE_COLOR);
cv::Mat ground_mat = cv::imread("../resources/ground.jpg", CV_LOAD_IMAGE_COLOR);
cv::Mat elder_mat = cv::imread("../resources/elder2.png", CV_LOAD_IMAGE_COLOR);
cv::Mat elder1_mat = cv::imread("../resources/elder1.jpg", CV_LOAD_IMAGE_COLOR);

Face fArray[max_face];
Vec vxArray[max_vx];
Vec vnArray[max_vx];
Vec vtArray[max_vx];
Vec fnArray[max_face];
Vec ilArray[max_illu];
Vec liArray[max_illu];
size_t l_face, l_vertex, l_normal, l_texture;

KDTree *tree;
Scene scene;
Image img(Vec(0, -5, -5), Vec(0, 1, 1), 1);

std::atomic<int> cnt_pixels;

using json = nlohmann::json ;
json js;

/*
 * ETA
 */
inline float eta(int progress, int total,
        const std::chrono::high_resolution_clock::time_point &start,
        const std::chrono::high_resolution_clock::time_point &now)
{
    assert(progress != 0);
    return (float) ((1. * (total - progress) / progress) * ((now - start).count() / 1e9));
}

inline void load_config()
{
    fprintf(stderr, "Loading scene config... \n");
    auto start = std::chrono::high_resolution_clock::now();
    std::ifstream fin(std::string("../") + config_name);
    fin >> js;
    fin.close();
    auto now = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "Load successful in %.3fs.\n", (now - start).count() / 1e9);
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
    min_z -= view_dis * len_z;

    max_x += view_dis * len_x;
    max_y += view_dis * len_y;
    max_z += view_dis * len_z;

    scene.area = (max_z - min_z) * (max_x - min_x);

    // Set the illumination;
    if (!enable_global)
    {
        ADD_ILLU(WHITE, max_x - len_x / 2, max_y - len_y / 2, max_z - len_z / 2);
        ADD_ILLU(WHITE, max_x - len_x / 2, max_y - len_y / 2, min_z + len_z / 2);
        // Set multi-illumination to enable soft shadow.
        if (enable_shadow) {
            for (int i = -1; i < 2; ++i)
                for (int j = -1; j < 2; ++j)
                    for (int k = -1; k < 2; ++k) {
                        if (i == 0 && j == 0 && k == 0) continue;
                        ADD_ILLU(WHITE, (float) (max_x - len_x * (.5 + 5e-2 * i)),
                                 (float) (max_y - len_y * (.5 + 5e-2 * j)), (float) (max_z - len_z * (.5 + 5e-2 * k)));
                    }

            for (int i = -1; i < 2; ++i)
                for (int j = -1; j < 2; ++j)
                    for (int k = -1; k < 2; ++k) {
                        if (i == 0 && j == 0 && k == 0) continue;
                        ADD_ILLU(WHITE, (float) (max_x - len_x * (.5 + 5e-2 * i)),
                                 (float) (max_y - len_y * (.5 + 5e-2 * j)), (float) (min_z + len_z * (.5 + 5e-2 * k)));
                    }
        }
    }
    else
    {
        ADD_CORD(min_x + .5 * view_dis * len_x, max_y - eps, min_z + .5 * view_dis * len_z);
        ADD_CORD(min_x + .5 * view_dis * len_x, max_y - eps, max_z - .5 * view_dis * len_z);
        ADD_CORD(max_x - .5 * view_dis * len_x, max_y - eps, max_z - .5 * view_dis * len_z);
        ADD_CORD(max_x - .5 * view_dis * len_x, max_y - eps, min_z + .5 * view_dis * len_z);
        int idx = (int) scene.size_vx,
            idt = 0;
        Material
            light(Vec(.2, .2, .2), Vec(.4, .4, .4), Vec(.4, .4, .4), nullptr, Vec(WHITE), Vec(WHITE), DIFF);
        ADD_WALL(4, 3, 2, 1, 1, 1, 1, 1, light);
    }
    // Change the camera's view point.
    img.cam.d = Vec(-(2 + view_dis), -(1 + view_dis), (2 + view_dis)).norm();
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

    // Add some texture coordinates to vt list;
    ADD_TEX_CORD(-3, -3);
    ADD_TEX_CORD(-3, 3);
    ADD_TEX_CORD(3, 3);
    ADD_TEX_CORD(3, -3);

    int idx = (int) scene.size_vx,
        idt = (int) scene.size_vt;
    Material
            wall(Vec(.2, .2, .2), Vec(.4, .4, .4), Vec(.4, .4, .4), &wall_mat),
            ground(Vec(.2, .2, .2), Vec(.4, .4, .4), Vec(.4, .4, .4), &ground_mat),
            elder(Vec(.2, .2, .2), Vec(.4, .4, .4), Vec(.4, .4, .4), &elder_mat),
            elder1(Vec(.2, .2, .2), Vec(.4, .4, .4), Vec(.4, .4, .4), &elder1_mat);

    if (enable_global)  // Set the property of wall when global illumination is enabled.
    {
        wall.refl   = DIFF;
        ground.refl = DIFF;
        elder.refl  = DIFF;
        elder1.refl = SPEC;

        wall.c      = Vec(WHITE) * .7;
        ground.c    = Vec(WHITE) * .7;
        elder.c     = Vec(WHITE) * .7;
        elder1.c    = Vec(WHITE) * .7;
    }

    ADD_WALL(6, 5, 7, 8, 4, 3, 2, 1, elder); // elder
    ADD_WALL(4, 3, 1, 2, 4, 3, 2, 1, wall);
    ADD_WALL(8, 7, 3, 4, 4, 3, 2, 1, ground);
    ADD_WALL(2, 1, 5, 6, 4, 3, 2, 1, wall);
    ADD_WALL(4, 2, 6, 8, 4, 3, 2, 1, wall);
    ADD_WALL(7, 5, 1, 3, 4, 3, 2, 1, elder1); // elder1
    return ;
}

void load_and_construct_scene()
{
    auto start = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "Loading model... \n");
    for (int i = 0; i < js["objects"].size(); ++i) {
        json model = js["objects"][i];
        obj_loader((char *) ("../resources/" + model["name"].get<std::string>() + ".obj").c_str(), fArray, vnArray, vxArray, vtArray,
                   l_face, l_vertex, l_normal, l_texture, model);
    }
    scene.f_array   = fArray;
    scene.vn_array  = vnArray;
    scene.vx_array  = vxArray;
    scene.vt_array  = vtArray;
    scene.fn_array  = fnArray;
    scene.il_array  = ilArray;
    scene.li_array  = liArray;
    scene.size_f    = l_face;
    scene.size_vn   = l_normal;
    scene.size_vx   = l_vertex;
    scene.size_vt   = l_texture;
    scene.size_il   = 0;

    add_wall_illumination();

    // Compute the normal vector.
    for (int i = 0; i < scene.size_f; ++i)
    {
        Vec &v = scene.fn_array[i];
        for (int j = 0; j < scene.f_array[i].get_size(); ++j)
            v = v + (scene.vx_array[scene.f_array[i].get_elem_idxV(j)] %
                    scene.vx_array[scene.f_array[i].get_elem_idxV(j + 1)]);
        scene.f_array[i].area = (float) sqrt(v.dot(v)) / 2;
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
                        for (int s = 0; s < num_samples; ++s) {
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
                                r = r + global_ill(Ray(img.cam.o, d.norm()), 0, scene) * (1. / num_samples);
                            else
                                r = r + local_ill(Ray(img.cam.o, d.norm()), scene) * (1. / num_samples);
                        }
                        col = col + r * .25;
                    }
            }
            else
            {
                for (int s = 0; s < num_samples; ++s) {
                    Vec
                            d = img.cx * (1. * x / width - .5) +
                                img.cy * (1. * y / height - .5) + img.cam.d;
                    if (enable_global)
                        col = col + global_ill(Ray(img.cam.o, d.norm()), 0, scene) * (1. / num_samples);
                    else
                        col = col + local_ill(Ray(img.cam.o, d.norm()), scene) * (1. / num_samples);
                }
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
                        for (int s = 0; s < num_samples; ++s) {
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
                                r = r + global_ill(Ray(img.cam.o, d.norm()), 0, scene) * (1. / num_samples);
                            else
                                r = r + local_ill(Ray(img.cam.o, d.norm()), scene) * (1. / num_samples);
                        }
                        col = col + r * .25;
                    }
            }
            else
            {
                for (int s = 0; s < num_samples; ++s) {
                    Vec
                            d = img.cx * (1. * x / width - .5) +
                                img.cy * (1. * y / height - .5) + img.cam.d;
                    if (enable_global)
                        col = col + global_ill(Ray(img.cam.o, d.norm()), 0, scene);
                    else
                        col = col + local_ill(Ray(img.cam.o, d.norm()), scene);
                }
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

    if (enable_display)
    {
        cv::namedWindow("Display scene", cv::WINDOW_AUTOSIZE);
        cv::imshow("Display scene", img.to_cv2_pixel());
        cvWaitKey(0);
    }

    return ;
}

void parse_argument(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--config") == 0) {
            strcpy(config_name, argv[++i]);
        } else if (strcmp(argv[i], "--anti_aliasing") == 0) {
            enable_anti_aliasing = true;
        } else if (strcmp(argv[i], "--shadow") == 0) {
            enable_shadow = true;
        } else if (strcmp(argv[i], "--sah") == 0) {
            enable_sah = true;
        } else if (strcmp(argv[i], "--distance") == 0) {
            view_dis = (float) atof(argv[++i]);
        } else if (strcmp(argv[i], "--display") == 0) {
            enable_display = true;
        } else if (strcmp(argv[i], "--global") == 0) {
            enable_global = true;
        } else if (strcmp(argv[i], "--core") == 0) {
            num_workers = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--samples") == 0) {
            num_samples = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--help") == 0)
        {
            fprintf(stdout, "Gallifrey, a naive 3D engine.\n");
            fprintf(stdout, "--config CONFIG_NAME:\t specifies scene configuration file name.\n");
            fprintf(stdout, "--sah   ENABLE_SAH:\t specifies whether to use SAH KD Tree or SPACE MEDIUM KD Tree\n");
            fprintf(stdout, "--distance DISTANCE:\t specifies the distance between the camera and the object.\n");
            fprintf(stdout, "--samples SAMPLES:\t specifies the number of samples in MCPT.\n");
            fprintf(stdout, "--core CORE:\t\t specifies the number of cores this program uses\n");
            fprintf(stdout, "--anti_aliasing:\t specifies whether to enable anti aliasing.\n");
            fprintf(stdout, "--shadow:\t\t specifies whether to enable (soft) shadow.\n");
            fprintf(stdout, "--global:\t\t specifies whether to enable global illumination.\n");
            fprintf(stdout, "--display:\t\t specifies whether to display the picture right now.\n");
            fprintf(stdout, "--help:\t\t\t display this information.\n");
            fprintf(stdout, "Please refer to README.md for more details.\n");
        } else {
            fprintf(stderr, "Argument error, please use %s --help to show help.\n", argv[0]);
            exit(1);
        }
    }
    if (strlen(config_name) == 0)
    {
        fprintf(stderr, "Configuration name must be specified, please use %s --help to show help.\n", argv[0]);
        exit(1);
    }
    return ;
}

int main(int argc, char *argv[])
{
    // Initialization
    memset(config_name, '\0', sizeof(config_name));
#ifdef DEBUG
    enable_anti_aliasing = true;
    enable_shadow = false;
    enable_global = true;
    enable_sah = true;
    enable_display = true;
    num_samples = 20;
    view_dis = 1;
    strcpy(config_name, "config.json");
#else
    parse_argument(argc, argv);
#endif

    assert(!enable_global or !enable_shadow);
    assert(enable_global or (num_samples == 1));

    // unit test.
    test_dump_image();
    test_intersection();
    test_aabb();

    // main.
    load_config();
    load_and_construct_scene();
    tree = new KDTree(&scene);
    tree->build_tree(enable_sah);
    rendering();
    dump_image();
    fprintf(stderr, "DONE.\n");
    return 0;
}
