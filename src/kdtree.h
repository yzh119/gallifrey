//
// Created by zihao on 2017/3/26.
//

#ifndef GALLIFREY_KDTREE_H
#define GALLIFREY_KDTREE_H

//#include <bits/shared_ptr.h>
#include <memory>
#include <vector>
#include <chrono>
#include <algorithm>
#include "aabb.h"

class KDTree {
private:
    bool flag;
    class node
    {
    public:
        node (const std::vector<int> &elems, const Box &box): elems(elems), box(box) {
            ptl = ptr = nullptr;
        }
        ~node () {}
        std::vector<int> elems;
        Box box;
        std::shared_ptr <node> ptl, ptr;
    };
    Scene *s;
    std::chrono::high_resolution_clock::time_point start;
    void recursive_build_kd_node(std::shared_ptr<node> &v, int depth);
    std::vector<Vec> fmin, fmax;
public:
    KDTree(Scene *s): flag(false), s(s), start(std::chrono::high_resolution_clock::now()) {
        fprintf(stderr, "Building SAH-KDTree...\n");
        Vec vmin(1e8, 1e8, 1e8), vmax((float) -1e8, (float) -1e8, (float) -1e8);

        // Here s->size_f - 6 means we don't take the bounding into account.
        for (int i = 0; i < s->size_f; ++i)
        {
            Vec current_vmin(1e8, 1e8, 1e8), current_vmax((float) -1e8, (float) -1e8, (float) -1e8);
            for (int j = 0; j < s->f_array[i].get_size(); ++j) {
                int v = s->f_array[i].get_elem_idxV(j);
                if (s->vx_array[v].x < current_vmin.x) current_vmin.x = s->vx_array[v].x;
                if (s->vx_array[v].y < current_vmin.y) current_vmin.y = s->vx_array[v].y;
                if (s->vx_array[v].z < current_vmin.z) current_vmin.z = s->vx_array[v].z;
                if (s->vx_array[v].x > current_vmax.x) current_vmax.x = s->vx_array[v].x;
                if (s->vx_array[v].y > current_vmax.y) current_vmax.y = s->vx_array[v].y;
                if (s->vx_array[v].z > current_vmax.z) current_vmax.z = s->vx_array[v].z;
            }
            current_vmin.x -= eps; current_vmax.x += eps;
            current_vmin.y -= eps; current_vmax.y += eps;
            current_vmin.z -= eps; current_vmax.z += eps;
            fmin.push_back(current_vmin);
            fmax.push_back(current_vmax);
            if (vmin.x > current_vmin.x) vmin.x = current_vmin.x;
            if (vmin.y > current_vmin.y) vmin.y = current_vmin.y;
            if (vmin.z > current_vmin.z) vmin.z = current_vmin.z;

            if (vmax.x < current_vmax.x) vmax.x = current_vmax.x;
            if (vmax.y < current_vmax.y) vmax.y = current_vmax.y;
            if (vmax.z < current_vmax.z) vmax.z = current_vmax.z;
        }

        std::vector<int> all_faces;
        // s->size_f - 6: for the same reason above.
        for (int i = 0; i < s->size_f; ++i)
            all_faces.emplace_back(i);

        root = std::make_shared<node> (all_faces, Box(vmin, vmax));
    }
    ~KDTree() {}

    std::shared_ptr <node> root;
    inline void build_tree();
    bool get_intersection(std::shared_ptr<node> &v, const Ray &r, float &t, int &id);
    inline float cost(float sa, int na, float sb, int nb) const ;
    inline float surface_area(const Vec &min, const Vec &max) const ;
};

extern KDTree *tree;

/*
 * To build the SAH KDTree by using the algorithm that takes O(n log^2 n) time;
 */
inline void KDTree::build_tree() {
    flag = true;
    recursive_build_kd_node(root, 0);
    auto now = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "Building successful in %.3fs.\n", (now - start).count() / 1e9);
}

/*
 * Recursively build the KD-Tree.
 */
void KDTree::recursive_build_kd_node(std::shared_ptr<KDTree::node> &v, int depth) {
    if (depth == 100) return ;                  // If too deep, prune it.
    if (v->elems.size() <= 20) return ;         // If too small, prune it.
    bool splix_x, split_y, split_z = splix_x = split_y = false;
    int k_x, k_y, k_z = k_x = k_y = -1;
    float minc = 1e8, current;

    std::vector<std::pair<std::pair<int, int>, float> > x_sort, y_sort, z_sort;
    for (int i = 0; i < v->elems.size(); ++i)
    {
        x_sort.push_back(std::make_pair(std::make_pair(i, 0), fmin[v->elems[i]].x));
        x_sort.push_back(std::make_pair(std::make_pair(i, 1), fmax[v->elems[i]].x));
        y_sort.push_back(std::make_pair(std::make_pair(i, 0), fmin[v->elems[i]].y));
        y_sort.push_back(std::make_pair(std::make_pair(i, 1), fmax[v->elems[i]].y));
        z_sort.push_back(std::make_pair(std::make_pair(i, 0), fmin[v->elems[i]].z));
        z_sort.push_back(std::make_pair(std::make_pair(i, 1), fmax[v->elems[i]].z));
    }

    auto comp = [](const std::pair<std::pair<int, int>, float> &a, const std::pair<std::pair<int, int>, float> &b)
    {
        return a.second < b.second;
    };
    std::sort(x_sort.begin(), x_sort.end(), comp);
    std::sort(y_sort.begin(), y_sort.end(), comp);
    std::sort(z_sort.begin(), z_sort.end(), comp);

    std::vector<std::pair<Vec, Vec> > reversed(x_sort.size());
    // To test whether it's best to split x axis;
    reversed[x_sort.size() - 1] = std::make_pair(Vec(x_sort.back().first.first), Vec());
    for (int i = x_sort.size() - 1; i > 0; i--)
    {
        reversed[i] =
    }
    for (int i = 0; i < x_sort.size() - 1; ++i)
    {

    }

    // To test whether it's best to split y axis;

    // To test whether it's best to split z axis;
    // TODO

    if (splix_x)                // split_x
    {

    } else if (split_y)         // split_y
    {

    } else                      // split_z
    {

    }

    return ;
}

/*
 * To get the intersection of a ray and a bunch of faces.
 */
bool KDTree::get_intersection(std::shared_ptr<KDTree::node> &v, const Ray &r, float &t, int &id) {
    assert(!(v->ptl == nullptr xor v->ptr == nullptr));
    if (v->ptl == nullptr && v->ptr == nullptr)
    {
        for (int i = 0; i < v->elems.size(); ++i)
        {
            int current_id = v->elems[i];
            float d = intersect_with_face(r, s->f_array[current_id], s->vx_array);
            if (d > eps)
            {
                if (d < t)
                {
                    t = d;
                    id = current_id;
                }
            }
        }
        return t > eps;
    }

    bool ret = false;
    if (v->ptl->box.intersect_with_ray(r, eps, t))
        ret |= get_intersection(v->ptl, r, t, id);

    if (v->ptr->box.intersect_with_ray(r, eps, t))
        ret |= get_intersection(v->ptr, r, t, id);

    return ret;
}

/*
 * To compute the heuristic cost.
 */
inline float KDTree::cost(float sa, int na, float sb, int nb) const{
    return sa * na + sb * nb;
}

inline float KDTree::surface_area(const Vec &min, const Vec &max) const {
    float
        x = max.x - min.x,
        y = max.y - min.y,
        z = max.z - min.z;
    return (float) (2.0 * (x * y + y * z + z * x));
}

/*
 * INTERSECTS RAY WITH SCENE
 * Using SHA-KD Tree api;
 */
inline bool high_level_intersect(const Ray &r, float &t, int &id, const Scene &s)
{
    return tree->get_intersection(tree->root, r, t, id);
}

#endif //GALLIFREY_KDTREE_H
