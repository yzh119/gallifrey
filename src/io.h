//
// Created by zihao on 2017/3/23.
//

#ifndef GALLIFREY_IO_H
#define GALLIFREY_IO_H

#include "bitmap_image.hpp"
#include "geometry.h"
#include <cstring>
#include <cstdlib>
#include <climits>
#include <cctype>
#include "json.hpp"
#define YELLOW  1,1,0
#define WHITE   1,1,1
#define RED     1,0,0
#define GREEN   0,1,0
#define CYAN    0,1,1
#define BLUE    0,0,1
#define PINK    1,0,1

const unsigned int width = 640;
const unsigned int height = 480;

extern bool enable_global;

using json = nlohmann::json ;

/*
 * Generate .bmp file from pixels.
 */
inline void dump_bitmap(char *path, uint8_t *col)
{
    bitmap_image dump(width, height);
    for (unsigned int y = 0; y < height; ++y)
        for (unsigned int x = 0; x < width; ++x)
        {
            int pos = y * width + x;
            rgb_t c;
            c.red   = col[3 * pos];
            c.green = col[3 * pos + 1];
            c.blue  = col[3 * pos + 2];
            dump.set_pixel(x, y, c);
        }
    dump.save_image(path);
    return;
}

/*
 * Generate noise picture to test 'output_to_bmp'.
 * Just for test.
 */
inline void fill_pic_with_noise(uint8_t *col)
{
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            int pos = y * width + x;
            col[3 * pos]        = (uint8_t) (0xff * (1. * rand() / RAND_MAX));
            col[3 * pos + 1]    = (uint8_t) (0xff * (1. * rand() / RAND_MAX));
            col[3 * pos + 2]    = (uint8_t) (0xff * (1. * rand() / RAND_MAX));
        }
}

/*
 * Test the dumpfile function.
 */
inline void test_dump_image()
{
    uint8_t *col;
    col = new uint8_t[3 * width * height];
    fill_pic_with_noise(col);
    dump_bitmap((char *) "../out/test.bmp", col);
    delete col;
}

/*
 * Get next str token from buffer.
 */
static inline char *get_next_str(char *buffer, char *target) {
    if (*buffer == '\0') return buffer;
    while (isblank(*buffer)) ++buffer;
    char *ptr = buffer;
    while (!isblank(*ptr) && *ptr != '\n' && *ptr != '\r' && *ptr != 0) ++ptr;
    strncpy(target, buffer, ptr - buffer);
    target[ptr - buffer] = '\0';
    return ptr;
}

/*
 * Get next float token from buffer.
 */
static inline char *get_next_float(char *buffer, float &target) {
    if (*buffer == '\0') return buffer;
    while (isblank(*buffer)) ++buffer;
    char *ptr = buffer;
    while (isdigit(*ptr) || *ptr == 'e' || *ptr == 'E' || *ptr == '-' || *ptr == '+' || *ptr == '.') ++ptr;
    char str_target[20];
    strncpy(str_target, buffer, ptr - buffer);
    str_target[ptr - buffer] = '\0';
    target = (float) atof(str_target);
    return ptr;
}

/*
 * Get next integer token from buffer.
 */
static inline char *get_next_int(char *buffer, int &target) {
    if (*buffer == '\0') return buffer;
    while (isblank(*buffer)) ++buffer;
    char *ptr = buffer;
    while (isdigit(*ptr) || *ptr == '-') ++ptr;
    char str_target[20];
    strncpy(str_target, buffer, ptr - buffer);
    str_target[ptr - buffer] = '\0';
    target = atoi(str_target);
    return ptr;
}

/*
 * Extract information from .obj "face" elements.
 */
static inline char *parse_face(char *ptr, int &idx_v, int &idx_vt, int &idx_vn)
{
    while (isblank(*ptr)) ++ptr;
    ptr = get_next_int(ptr, idx_v);

    if (isblank(*ptr) || *ptr == '\r' || *ptr == '\n' || *ptr == '\0')
    {
        while (isblank(*ptr)) ptr++;
        return ptr;
    }
    ++ptr;

    if (isdigit(*ptr))
    {
        ptr = get_next_int(ptr, idx_vt);
    }
    else
    {
        ++ptr;
        idx_vt = 0;
    }

    if (isblank(*ptr) || *ptr == '\r' || *ptr == '\n' || *ptr == '\0')
    {
        while (isblank(*ptr)) ptr++;
        return ptr;
    }
    if (isdigit(*ptr))
    {
        ptr = get_next_int(ptr, idx_vn);
    }
    else
    {
        ++ptr;
        idx_vn = 0;
    }

    while (isblank(*ptr)) ptr++;
    return ptr;
}

/*
 * Extract information from .obj file.
 */
static inline void parse_obj(char *buffer, Face *f_array, Vec *vn_array, Vec *vx_array, Vec *vt_array, size_t &l_f, size_t &l_vn, size_t &l_vx, size_t &l_vt, json &config)
{
    size_t offset_vx = l_vx, offset_vn = l_vn, offset_vt = l_vt;
    auto offset = config["offset"].get<std::vector<float> >();
    auto scale = config["scale"].get<float> ();
    auto emission = config["e"].get<std::vector<float> >();
    auto color = config["c"].get<std::vector<float> >();
    auto refl_t = config["refl"].get<std::string>();
    auto ka = config["ka"].get<std::vector<float> > ();
    auto ks = config["ks"].get<std::vector<float> > ();
    auto kd = config["kd"].get<std::vector<float> > ();

    char *ptr = buffer;
    while (*buffer)
    {
        if (isblank(*buffer)) ++buffer;
        if (*buffer == '#')
        {
            ptr = buffer;
            while (*ptr != '\n' && *ptr && *ptr != '\r') ++ptr;
            if (*ptr && *ptr == '\r') ++ptr;
            if (*ptr && *ptr == '\n') ++ptr;
        }
        else
        {
            char type[10];
            ptr = get_next_str(buffer, type);
            if (strcmp(type, "v") == 0)
            {
                float x, y, z, w = 0;
                ptr = get_next_float(ptr, x);
                ptr = get_next_float(ptr, y);
                ptr = get_next_float(ptr, z);
                ptr = get_next_float(ptr, w);
                x += offset[0];
                z += offset[1];
                vx_array[l_vx++].set_coordinate(x * scale, y * scale, z * scale);
            }
            else if (strcmp(type, "vn") == 0)
            {
                float x, y, z;
                ptr = get_next_float(ptr, x);
                ptr = get_next_float(ptr, y);
                ptr = get_next_float(ptr, z);
                vn_array[l_vn++].set_coordinate(x * scale, y * scale, z * scale);
            }
            else if (strcmp(type, "vt") == 0)
            {
                float u, v, w = 0;
                ptr = get_next_float(ptr, u);
                ptr = get_next_float(ptr, v);
                ptr = get_next_float(ptr, w);
                vt_array[l_vt++].set_coordinate(u, v, 0);
            }
            else if (strcmp(type, "f") == 0)
            {
                int idx_v = 0, idx_vt = 0, idx_vn = 0;
                while (isdigit(*(ptr = parse_face(ptr, idx_v, idx_vt, idx_vn))))
                {
                    idx_v += (idx_v > 0) * offset_vx;
                    idx_vt += (idx_vt > 0) * offset_vt;
                    idx_vn += (idx_vn > 0) * offset_vn;
                    f_array[l_f].add_vx(idx_v - 1, idx_vt - 1, idx_vn - 1);
                }
                idx_v += (idx_v > 0) * offset_vx;
                idx_vt += (idx_vt > 0) * offset_vt;
                idx_vn += (idx_vn > 0) * offset_vn;
                f_array[l_f].add_vx(idx_v - 1, idx_vt - 1, idx_vn - 1);

                if (!enable_global)
                {
                    f_array[l_f].set_ka(Vec(ka[0], ka[1], ka[2]));
                    f_array[l_f].set_kd(Vec(kd[0], kd[1], kd[2]));
                    f_array[l_f].set_ks(Vec(ks[0], ks[1], ks[2]));
                    l_f++;
                }
                else
                {
                    f_array[l_f].set_c(Vec(color[0], color[1], color[2]));
                    if (refl_t == "DIFF"){
                        f_array[l_f].set_refl(DIFF);
                    } else if (refl_t == "SPEC") {
                        f_array[l_f].set_refl(SPEC);
                    } else if (refl_t == "REFR") {
                        f_array[l_f].set_refl(REFR);
                    } else {
                        fprintf(stderr, "Config Error!");
                        exit(1);
                    }
                    f_array[l_f].set_e(Vec(emission[0], emission[1], emission[2]));
                    l_f++;
                }
            }
            else if (strcmp(type, "g") == 0)
            {
                char str[20];
                ptr = get_next_str(ptr, str);
            }
            else if (strcmp(type, "s") == 0)
            {
                int s = 0;
                ptr = get_next_int(ptr, s);
            }
            else if (strcmp(type, "mtllib") == 0)
            {
                char str[20];
                ptr = get_next_str(ptr, str);
            }
            else if (strcmp(type, "o") == 0)
            {
                char str[20];
                ptr = get_next_str(ptr, str);
            }
            else if (strcmp(type, "usemtl") == 0)
            {
                char str[20];
                ptr = get_next_str(ptr, str);
            }
        }
        if (*ptr && *ptr == '\r')
            ++ptr;

        if (*ptr && *ptr == '\n')
            ++ptr;

        buffer = ptr;
    }
}

/*
 * Import .obj files from disk.
 *
 * OBJ file format (from enwiki):
 * # List of geometric vertices, with (x,y,z[,w]) coordinates, w is optional and defaults to 1.0.
 * v 0.123 0.234 0.345 1.0
 * v ...
 * ...
 * # List of texture coordinates, in (u, v [,w]) coordinates, these will vary between 0 and 1, w is optional and defaults to 0.
 * vt 0.500 1 [0]
 * vt ...
 * ...
 * # List of vertex normals in (x,y,z) form; normals might not be unit vectors.
 * vn 0.707 0.000 0.707
 * vn ...
 * ...
 * # Parameter space vertices in ( u [,v] [,w] ) form; free form geometry statement ( see below )
 * vp 0.310000 3.210000 2.100000
 * vp ...
 * ...
 * # Polygonal face element (see below)
 * f 1 2 3
 * f 3/1 4/2 5/3
 * f 6/4/1 3/5/3 7/6/5
 * f 7//1 8//2 9//3
 * f ...
 * ...
 */

void inline obj_loader(char *path, Face *f_array, Vec *vn_array, Vec *vx_array, Vec *vt_array, size_t &l_f, size_t &l_vx, size_t &l_vn, size_t &l_vt, json &config)
{
    FILE *pFile;
    char *buffer;
    size_t result, lSize;

    pFile = fopen (path, "rb");

    if (pFile == NULL)
    {
        fputs("File error", stderr);
        exit(1);
    }

    fseek(pFile, 0, SEEK_END);
    lSize = (size_t) ftell(pFile);
    rewind(pFile);

    buffer = new char[lSize + 1];

    result = fread(buffer, 1, lSize, pFile);
    buffer[lSize] = '\0';
    if (result != lSize)
    {
        fputs ("Reading error", stderr);
        exit(3);
    }

    parse_obj(buffer, f_array, vn_array, vx_array, vt_array, l_f, l_vn, l_vx, l_vt, config);

    // terminate
    fclose (pFile);
    delete buffer;
    return ;
}


#endif //GALLIFREY_IO_H
