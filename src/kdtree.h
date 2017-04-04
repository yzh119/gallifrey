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


inline Vec vec_max(const Vec &a, const Vec &b)
{
    return Vec(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
}

inline Vec vec_min(const Vec &a, const Vec &b)
{
    return Vec(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
}

class KDTree {
private:
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
    bool flag;
    Scene *s;
    std::chrono::high_resolution_clock::time_point start;
    void recursive_build_kd_node(std::shared_ptr<node> &v, int depth);
    std::vector<Vec> fmin, fmax;
public:
    KDTree(Scene *s): flag(false), s(s), start(std::chrono::high_resolution_clock::now()) {
        fprintf(stderr, "Building SAH-KDTree...\n");
        Vec vmin(1e8, 1e8, 1e8), vmax((float) -1e8, (float) -1e8, (float) -1e8);

        for (int i = 0; i < s->size_f; ++i)
        {
            Vec current_vmin(1e8, 1e8, 1e8), current_vmax((float) -1e8, (float) -1e8, (float) -1e8);
            for (int j = 0; j < s->f_array[i].get_size(); ++j) {
                int v = s->f_array[i].get_elem_idxV(j);
                current_vmin = vec_min(current_vmin, s->vx_array[v]);
                current_vmax = vec_max(current_vmax, s->vx_array[v]);
            }
            current_vmin.x -= eps; current_vmax.x += eps;
            current_vmin.y -= eps; current_vmax.y += eps;
            current_vmin.z -= eps; current_vmax.z += eps;
            fmin.push_back(current_vmin);
            fmax.push_back(current_vmax);

            vmin = vec_min(current_vmin, vmin);
            vmax = vec_max(current_vmax, vmax);
        }

        std::vector<int> all_faces;
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
    bool split_x, split_y, split_z = split_x = split_y = false;
    int k_x, k_y, k_z = k_x = k_y = -1;
    std::pair<Vec, Vec> box_x_l, box_x_r, box_y_l, box_y_r, box_z_l, box_z_r;
    float minc = 1e9, current_c;                // minc is the min cost, and current_c is the current cost.

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
    std::vector<int> N_reversed(x_sort.size());
    // To test whether it's best to split x axis;
    reversed[x_sort.size() - 1] = std::make_pair(fmin[x_sort.back().first.first], fmax[x_sort.back().first.first]);
    N_reversed[x_sort.size() - 1] = 1;
    for (int i = (int) (x_sort.size() - 1); i > 0; i--)
    {
        reversed[i] = reversed[i + 1];
        N_reversed[i] = N_reversed[i + 1];
        if (x_sort[i].first.second == 1)
        {
            reversed[i].first   = vec_min(reversed[i].first, fmin[x_sort[i].first.first]);
            reversed[i].second  = vec_max(reversed[i].first, fmax[x_sort[i].first.first]);
            ++N_reversed[i];
        }
    }

    std::pair<Vec, Vec> current = std::make_pair(fmin[x_sort.front().first.first], fmax[x_sort.front().first.first]);
    int N_current = 1;
    for (int i = 0; i < x_sort.size() - 1; ++i)
    {
        if (x_sort[i].first.second == 0)
        {
            current.first   = vec_min(current.first, fmin[x_sort[i].first.first]);
            current.second  = vec_max(current.first, fmax[x_sort[i].first.first]);
            ++N_current;
        }

        current_c = cost(
                surface_area(current.first, current.second), N_current,
                surface_area(reversed[i + 1].first, reversed[i + 1].second), N_reversed[i + 1]);

        if (current_c < minc)
        {
            minc = current_c;
            k_x = i;
            split_x = true;
            box_x_l = current;
            box_x_r = reversed[i + 1];
        }
    }

    // To test whether it's best to split y axis;
    reversed[y_sort.size() - 1] = std::make_pair(fmin[y_sort.back().first.first], fmax[y_sort.back().first.first]);
    N_reversed[y_sort.size() - 1] = 1;
    for (int i = (int) (y_sort.size() - 1); i > 0; i--)
    {
        reversed[i] = reversed[i + 1];
        N_reversed[i] = N_reversed[i + 1];
        if (y_sort[i].first.second == 1)
        {
            reversed[i].first   = vec_min(reversed[i].first, fmin[y_sort[i].first.first]);
            reversed[i].second  = vec_max(reversed[i].first, fmax[y_sort[i].first.first]);
            ++N_reversed[i];
        }
    }

    current = std::make_pair(fmin[y_sort.front().first.first], fmax[y_sort.front().first.first]);
    N_current = 1;
    for (int i = 0; i < y_sort.size() - 1; ++i)
    {
        if (y_sort[i].first.second == 0)
        {
            current.first   = vec_min(current.first, fmin[y_sort[i].first.first]);
            current.second  = vec_max(current.first, fmax[y_sort[i].first.first]);
            ++N_current;
        }

        current_c = cost(
                surface_area(current.first, current.second), N_current,
                surface_area(reversed[i + 1].first, reversed[i + 1].second), N_reversed[i + 1]);

        if (current_c < minc)
        {
            minc = current_c;
            k_y = i;
            split_x = false;
            split_y = true;
            box_y_l = current;
            box_y_r = reversed[i + 1];
        }
    }

    // To test whether it's best to split z axis;
    reversed[z_sort.size() - 1] = std::make_pair(fmin[z_sort.back().first.first], fmax[z_sort.back().first.first]);
    N_reversed[z_sort.size() - 1] = 1;
    for (int i = (int) (z_sort.size() - 1); i > 0; i--)
    {
        reversed[i] = reversed[i + 1];
        N_reversed[i] = N_reversed[i + 1];
        if (z_sort[i].first.second == 1)
        {
            reversed[i].first   = vec_min(reversed[i].first, fmin[z_sort[i].first.first]);
            reversed[i].second  = vec_max(reversed[i].first, fmax[z_sort[i].first.first]);
            ++N_reversed[i];
        }
    }

    current = std::make_pair(fmin[z_sort.front().first.first], fmax[z_sort.front().first.first]);
    N_current = 1;
    for (int i = 0; i < z_sort.size() - 1; ++i)
    {
        if (z_sort[i].first.second == 0)
        {
            current.first   = vec_min(current.first, fmin[z_sort[i].first.first]);
            current.second  = vec_max(current.first, fmax[z_sort[i].first.first]);
            ++N_current;
        }

        current_c = cost(
                surface_area(current.first, current.second), N_current,
                surface_area(reversed[i + 1].first, reversed[i + 1].second), N_reversed[i + 1]);

        if (current_c < minc)
        {
            minc = current_c;
            k_z = i;
            split_x = false;
            split_y = false;
            split_z = true;
            box_z_l = current;
            box_z_r = reversed[i + 1];
        }
    }
    // divide and conquer
    if (split_x)                // split_x
    {
        std::vector<int> v_left, v_right;
        for (int i = 0; i <= k_x; ++i)
            if (x_sort[i].first.second == 0)
                v_left.push_back(i);

        for (int i = k_x + 1; i < x_sort.size(); ++i)
            if (x_sort[i].first.second == 1)
                v_right.push_back(i);

        v->ptl = std::make_shared<node> (v_left, Box(box_x_l.first, box_x_l.second));
        v->ptr = std::make_shared<node> (v_right,Box(box_x_r.first, box_x_r.second));
    } else if (split_y)         // split_y
    {
        std::vector<int> v_left, v_right;
        for (int i = 0; i <= k_y; ++i)
            if (y_sort[i].first.second == 0)
                v_left.push_back(i);

        for (int i = k_y + 1; i < y_sort.size(); ++i)
            if (y_sort[i].first.second == 1)
                v_right.push_back(i);

        v->ptl = std::make_shared<node> (v_left, Box(box_y_l.first, box_y_l.second));
        v->ptr = std::make_shared<node> (v_right,Box(box_y_l.first, box_y_r.second));
    } else                      // split_z
    {
        std::vector<int> v_left, v_right;
        for (int i = 0; i <= k_z; ++i)
            if (z_sort[i].first.second == 0)
                v_left.push_back(i);

        for (int i = k_z + 1; i < z_sort.size(); ++i)
            if (z_sort[i].first.second == 1)
                v_right.push_back(i);

        v->ptl = std::make_shared<node> (v_left, Box(box_z_l.first, box_z_r.second));
        v->ptr = std::make_shared<node> (v_right,Box(box_z_r.first, box_z_r.second));
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

/*\
 * INTERSECTS RAY WITH SCENE
 * Using SHA-KD Tree api;
 */
inline bool high_level_intersect(const Ray &r, float &t, int &id, const Scene &s)
{
    return tree->get_intersection(tree->root, r, t, id);
}

#endif //GALLIFREY_KDTREE_H
