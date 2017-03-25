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

const unsigned int width = 640;
const unsigned int height = 480;

/*
 * Generate .bmp file from pixels.
 */
void dump_bitmap(char *path, uint8_t *col)
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
 */
void fill_pic_with_noise(uint8_t *col)
{
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            int pos = y * width + x;
            col[3 * pos]        = (uint8_t) (0xff * (1. * rand() / RAND_MAX));
            col[3 * pos + 1]    = (uint8_t) (0xff * (1. * rand() / RAND_MAX));
            col[3 * pos + 1]    = (uint8_t) (0xff * (1. * rand() / RAND_MAX));
        }
}


static char *get_next_int(char *buffer, int &target) {
    if (*buffer == '\0') return buffer;
    while (isblank(*buffer)) ++buffer;

    char *ptr = buffer;
    bool neg = false;
    if (*ptr == '-')
    {
        neg = true;
        ++ptr;
    }
    while (isdigit(*ptr)) ++ptr;
    char str_target[20];
    strncpy(str_target, buffer, ptr - buffer);
    str_target[ptr - buffer] = '\0';
    target = atoi(str_target);
    if (neg)
    {
        target = -target;
    }
    return ptr;
}

/*
 * Extract information from .obj "face" elements.
 */
static char *parse_face(char *ptr, int &idxV, int &idxVt, int &idxVn)
{
    if (*ptr == '\n') return ptr;
    if (*ptr == '\0') return ptr;
    while (isblank(*ptr)) ptr++;
    printf("%s\n", ptr);
    ptr = get_next_int(ptr, idxV);
    if (isblank(*ptr))
    {
        return ptr;
    }
    ++ptr;

    if (isblank(*ptr))
    {
        return ptr;
    }
    if (isdigit(*ptr))
    {
        ptr = get_next_int(ptr, idxVt);
    }
    else
    {
        ++ptr;
        idxVt = 0;
    }

    if (isblank(*ptr))
    {
        while (isblank(*ptr)) ptr++;
        return ptr;
    }
    if (isdigit(*ptr))
    {
        ptr = get_next_int(ptr, idxVn);
    }
    else
    {
        ++ptr;
        idxVn = 0;
    }

    while (isblank(*ptr)) ptr++;
    return ptr;
}

static char *get_next_str(char *buffer, char *target) {
    if (*buffer == '\0') return buffer;
    while (isblank(*buffer)) ++buffer;
    char *ptr = buffer;
    while (isalpha(*ptr)) ++ptr;
    strncpy(target, buffer, ptr - buffer);
    target[ptr - buffer] = '\0';
    return ptr;
}

static char *get_next_float(char *buffer, float &target) {
    if (*buffer == '\0') return buffer;
    while (isblank(*buffer)) ++buffer;
    char *ptr = buffer;
    bool neg = false;
    if (*ptr == '-')
    {
        neg = true;
        ++ptr;
    }
    while (isdigit(*ptr) || *ptr == '.') ++ptr;
    char str_target[20];
    strncpy(str_target, buffer, ptr - buffer);
    str_target[ptr - buffer] = '\0';
    target = (float) atof(str_target);
    if (neg)
    {
        target = -target;
    }
    return ptr;
}

/*
 * Extract information from .obj file.
 */
static void parse_obj(char *buffer, Face *fArray, Vector *vnArray, Vertex *vxArray, size_t &lf, size_t &lvn, size_t &lvx)
{
    char *ptr = buffer;
    while (*buffer)
    {
        printf("%d %d %d\n", (int) lf, (int) lvn, (int) lvx);
        if (isblank(*buffer)) ++buffer;
        if (*buffer == '#')
        {
            ptr = buffer;
            while (*ptr != '\n' && *ptr) ++ptr;
            if (*ptr) ++ptr;
        }
        else
        {
            char type[10];
            ptr = get_next_str(buffer, type);
            if (strcmp(type, "v") == 0)
            {
                float x, y, z, w = (float) 0.;
                ptr = get_next_float(ptr, x);
                ptr = get_next_float(ptr, y);
                ptr = get_next_float(ptr, z);
                //if (sscanf(ptr, "%f%n", &w, &bytes_now) == 1)
                //{
                //    ptr += bytes_now;
                //}
                vxArray[lvx++] = Vertex(x, y, z, w);
            }
            else if (strcmp(type, "vn") == 0)
            {
                float x, y, z;
                ptr = get_next_float(ptr, x);
                ptr = get_next_float(ptr, y);
                ptr = get_next_float(ptr, z);
                vnArray[lvn++] = Vector(x, y, z);
            }
            else if (strcmp(type, "f") == 0)
            {
                int idxF = 0, idxVt = 0, idxVn = 0;
                while (isdigit(*(ptr = parse_face(ptr, idxF, idxVt, idxVn))))
                {
                    fArray[lf].add_vx(idxF, idxVt, idxVn);
                }
                fArray[lf++].add_vx(idxF, idxVt, idxVn);
            }
            else if (strcmp(type, "g") == 0){
                char str[20];
                ptr = get_next_str(ptr, str);
            }
        }
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

void obj_loader(char *path, Face *fArray, Vector *vnArray, Vertex *vxArray, size_t &lf, size_t &lvn, size_t &lvx)
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

    buffer = new char[lSize];

    result = fread(buffer, 1, lSize, pFile);
    if (result != lSize)
    {
        fputs ("Reading error", stderr);
        exit (3);
    }

    parse_obj(buffer, fArray, vnArray, vxArray, lf, lvn, lvx);

    // terminate
    fclose (pFile);
    delete buffer;
    return ;
}


#endif //GALLIFREY_IO_H
