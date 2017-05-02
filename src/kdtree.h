//
// Created by zihao on 2017/3/26.
//

#ifndef GALLIFREY_KDTREE_H
#define GALLIFREY_KDTREE_H

//#include <bits/shared_ptr.h>
#include <memory>
#include <vector>
#include <chrono>
#include <thread>
#include <algorithm>
#include <future>
#include "aabb.h"
#define PARALLEL_DEPTH_THRES 1

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
    void recursively_build_sah_kd_node(std::shared_ptr<node> &v, int depth);
    void recursively_build_mid_kd_node(std::shared_ptr<node> &v, int depth);
    std::vector<Vec> fmin, fmax;
public:
    KDTree(Scene *s): flag(false), s(s), start(std::chrono::high_resolution_clock::now()) {
        fprintf(stderr, "Building KDTree...\n");
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
    inline void build_tree(bool enable_sah);
    bool get_intersection(std::shared_ptr<node> &v, const Ray &r, float &t, int &id, int depth);
    static inline float cost(float sa, int na, float sb, int nb) ;
    static inline float surface_area(const Vec &min, const Vec &max) ;
    static inline float surface_area(const std::pair<Vec, Vec> ) ;
};

extern KDTree *tree;

/*
 * To build the SAH KDTree by using the algorithm that takes O(n log^2 n) time;
 */
inline void KDTree::build_tree(bool enable_sah) {
    flag = true;
    if (enable_sah)
        recursively_build_sah_kd_node(root, 0);
    else
        recursively_build_mid_kd_node(root, 0);

    auto now = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "Building successful in %.3fs.\n", (now - start).count() / 1e9);
}

/*
 * Recursively build the KD-Tree.
 */
void KDTree::recursively_build_sah_kd_node(std::shared_ptr<KDTree::node> &v, int depth) {
    if (depth == 100) return ;                  // If too deep, prune it.
    if (v->elems.size() <= 20) return ;         // If too small, prune it.

    bool parallel = depth < PARALLEL_DEPTH_THRES;

    int selected_dim = -1;
    int k = -1;
    std::pair<Vec, Vec> box_l, box_r;
    float minc = (float) 1e9, current_c;                // minc is the max cost, and current_c is the current cost.

    typedef std::pair<std::pair<int, int>, float> tuple;

    auto comp = [](const tuple &a, const std::pair<std::pair<int, int>, float> &b)
    {
        if (a.second == b.second) return a.first.second < b.first.second;
        return a.second < b.second;
    };

    auto gen_sorted = [&](int dim) {
        std::vector<tuple> ret;
        for (int i = 0; i < v->elems.size(); ++i)
            switch (dim) {
                case 0:
                    ret.push_back(std::make_pair(std::make_pair(v->elems[i], 0), fmin[v->elems[i]].x));
                    ret.push_back(std::make_pair(std::make_pair(v->elems[i], 1), fmax[v->elems[i]].x));
                    break;
                case 1:
                    ret.push_back(std::make_pair(std::make_pair(v->elems[i], 0), fmin[v->elems[i]].y));
                    ret.push_back(std::make_pair(std::make_pair(v->elems[i], 1), fmax[v->elems[i]].y));
                    break;
                case 2:
                    ret.push_back(std::make_pair(std::make_pair(v->elems[i], 0), fmin[v->elems[i]].z));
                    ret.push_back(std::make_pair(std::make_pair(v->elems[i], 1), fmax[v->elems[i]].z));
                default:break;
            }
        std::sort(ret.begin(), ret.end(), comp);
        return ret;
    };

    std::future<std::vector<tuple> > task[3];
    if (parallel) {
        for (int i = 0; i < 3; ++i)
            task[i] = std::async(std::launch::async, std::bind(gen_sorted, i));
    }

    std::vector<tuple> sorted[3];

    for (int i = 0; i < 3; ++i)
        sorted[i] = parallel ? task[i].get(): gen_sorted(i);

    for (int dim = 0; dim < 3; ++dim) {
        std::vector<std::pair<Vec, Vec> > reversed(sorted[dim].size());
        std::vector<int> N_reversed(sorted[dim].size());
        // To test whether it's best to split x axis;
        reversed[sorted[dim].size() - 1] = std::make_pair(fmin[sorted[dim].back().first.first], fmax[sorted[dim].back().first.first]);
        N_reversed[sorted[dim].size() - 1] = 1;
        for (int i = (int) (sorted[dim].size() - 2); i > 0; i--) {
            reversed[i] = reversed[i + 1];
            N_reversed[i] = N_reversed[i + 1];
            if (sorted[dim][i].first.second == 1) {
                reversed[i].first = vec_min(reversed[i].first, fmin[sorted[dim][i].first.first]);
                reversed[i].second = vec_max(reversed[i].second, fmax[sorted[dim][i].first.first]);
                ++N_reversed[i];
            }
        }

        std::pair<Vec, Vec> current = std::make_pair(fmin[sorted[dim].front().first.first],
                                                     fmax[sorted[dim].front().first.first]);
        int N_current = 0;
        for (int i = 0; i < sorted[dim].size() - 1; ++i) {
            if (sorted[dim][i].first.second == 0) {
                current.first = vec_min(current.first, fmin[sorted[dim][i].first.first]);
                current.second = vec_max(current.second, fmax[sorted[dim][i].first.first]);
                ++N_current;
            }

            current_c = cost(
                    surface_area(current), N_current,
                    surface_area(reversed[i + 1]), N_reversed[i + 1]);

            if (current_c < minc && N_current != v->elems.size() && N_reversed[i + 1] != v->elems.size()) {
                minc = current_c;
                k = i;
                selected_dim = dim;
                box_l = current;
                box_r = reversed[i + 1];
            }
        }
    }

    assert(k >= 0 && selected_dim >= 0);

    // divide and conquer
    std::vector<int> v_left, v_right;
    for (int i = 0; i <= k; ++i)
        if (sorted[selected_dim][i].first.second == 0)
            v_left.push_back(sorted[selected_dim][i].first.first);

    for (int i = k + 1; i < sorted[selected_dim].size(); ++i)
        if (sorted[selected_dim][i].first.second == 1)
            v_right.push_back(sorted[selected_dim][i].first.first);

    if (v_left.size() + v_right.size() > 1.5 * v->elems.size()) return;
    v->ptl = std::make_shared<node> (v_left,  Box(box_l.first, box_l.second));
    v->ptr = std::make_shared<node> (v_right, Box(box_r.first, box_r.second));

    if (!parallel) {
        recursively_build_sah_kd_node(v->ptl, depth + 1);
        recursively_build_sah_kd_node(v->ptr, depth + 1);
    } else {
        std::thread
                build_l ( [=] {recursively_build_sah_kd_node(v->ptl, depth + 1);}),
                build_r ( [=] {recursively_build_sah_kd_node(v->ptr, depth + 1);});
        build_l.join();
        build_r.join();
    }
    return ;
}

/*
 * To get the intersection of a ray and a bunch of faces.
 */
bool KDTree::get_intersection(std::shared_ptr<KDTree::node> &v, const Ray &r, float &t, int &id, int depth) {
    assert(!(v->ptl == nullptr xor v->ptr == nullptr));
    if (v->ptl == nullptr && v->ptr == nullptr)
    {
        bool flag = false;
        for (int i = 0; i < v->elems.size(); ++i)
        {
            int current_id = v->elems[i];
            assert(current_id < s->size_f);

            float d = intersect_with_face(r, s->f_array[current_id], s->vx_array);
            if (d > eps)
            {
                if (d < t)
                {
                    t = d;
                    id = current_id;
                    flag = true;
                }
            }
        }
        return flag;
    }

    bool ret = false;

    if (v->ptl->elems.size() < v->ptr->elems.size())
    {
        float t_min;
        if (v->ptl->box.intersect_with_ray(r, eps, t, t_min))
            if (t_min < t)
                ret |= get_intersection(v->ptl, r, t, id, depth + 1);
        if (v->ptr->box.intersect_with_ray(r, eps, t, t_min))
            if (t_min < t)
                ret |= get_intersection(v->ptr, r, t, id, depth + 1);
    }
    else
    {
        float t_min;
        if (v->ptr->box.intersect_with_ray(r, eps, t, t_min))
            if (t_min < t)
                ret |= get_intersection(v->ptr, r, t, id, depth + 1);
        if (v->ptl->box.intersect_with_ray(r, eps, t, t_min))
            if (t_min < t)
                ret |= get_intersection(v->ptl, r, t, id, depth + 1);
    }

    return ret;
}

/*
 * To compute the heuristic cost.
 */
inline float KDTree::cost(float sa, int na, float sb, int nb) {
    return sa * na + sb * nb;
}

inline float KDTree::surface_area(const Vec &min, const Vec &max) {
    float
        x = max.x - min.x,
        y = max.y - min.y,
        z = max.z - min.z;
    return (float) (2.0 * (x * y + y * z + z * x));
}

/*
 * Recursively build the KD-tree by selecting the mid point as splitting plane.
 */
void KDTree::recursively_build_mid_kd_node(std::shared_ptr<KDTree::node> &v, int depth) {
    if (depth == 100) return ;                  // If too deep, prune it.
    if (v->elems.size() <= 20) return ;         // If too small, prune it.

    std::pair<Vec, Vec> box_l, box_r;

    std::vector<std::pair<std::pair<int, int>, float> > sorted;
    switch (depth % 3)
    {
        case 0:
        {
            for (int i = 0; i < v->elems.size(); ++i)
            {
                sorted.push_back(std::make_pair(std::make_pair(v->elems[i], 0), fmin[v->elems[i]].x));
                sorted.push_back(std::make_pair(std::make_pair(v->elems[i], 1), fmax[v->elems[i]].x));
            }
            break;
        }
        case 1:
        {
            for (int i = 0; i < v->elems.size(); ++i)
            {
                sorted.push_back(std::make_pair(std::make_pair(v->elems[i], 0), fmin[v->elems[i]].y));
                sorted.push_back(std::make_pair(std::make_pair(v->elems[i], 1), fmax[v->elems[i]].y));
            }
            break;
        }
        case 2:
        {
            for (int i = 0; i < v->elems.size(); ++i)
            {
                sorted.push_back(std::make_pair(std::make_pair(v->elems[i], 0), fmin[v->elems[i]].z));
                sorted.push_back(std::make_pair(std::make_pair(v->elems[i], 1), fmax[v->elems[i]].z));
            }
            break;
        }
        default:break;
    }

    auto comp = [](const std::pair<std::pair<int, int>, float> &a, const std::pair<std::pair<int, int>, float> &b)
    {
        if (a.second == b.second) return a.first.second < b.first.second;
        return a.second < b.second;
    };
    std::sort(sorted.begin(), sorted.end(), comp);
    std::sort(sorted.begin(), sorted.end(), comp);
    std::sort(sorted.begin(), sorted.end(), comp);

    int l_split = (int) (sorted.size() / 2), r_split = l_split + 1;
    float split_plane = sorted[l_split].second;

    std::vector<int> v_left, v_right;
    box_l = std::make_pair(fmin[sorted.front().first.first], fmax[sorted.front().first.first]);
    box_r = std::make_pair(fmin[sorted.back().first.first], fmax[sorted.back().first.first]);
    for (int i = 0; i <= l_split; ++i)
    {
        if (sorted[i].first.second == 0)
        {
            v_left.push_back(sorted[i].first.first);
            box_l.first = vec_min(box_l.first, fmin[sorted[i].first.first]);
            box_l.second = vec_max(box_l.second, fmax[sorted[i].first.first]);
        }
    }
    for (int i = (int) (sorted.size() - 1); i >= r_split; --i)
    {
        if (sorted[i].first.second == 1)
        {
            v_right.push_back(sorted[i].first.first);
            box_r.first = vec_min(box_r.first, fmin[sorted[i].first.first]);
            box_r.second = vec_max(box_r.second, fmax[sorted[i].first.first]);
        }
    }

    if (v_left.size() + v_right.size() > 1.5 * v->elems.size()) return ;

    switch (depth % 3)
    {
        case 0:
            box_l.second.x = split_plane;
            box_r.first.x = split_plane;
            break;
        case 1:
            box_l.second.y = split_plane;
            box_r.first.y = split_plane;
            break;
        case 2:
            box_l.second.z = split_plane;
            box_r.first.z = split_plane;
            break;
        default:break;
    }

    v->ptl = std::make_shared<node> (v_left,  box_l);
    v->ptr = std::make_shared<node> (v_right, box_r);

    recursively_build_mid_kd_node(v->ptl, depth + 1);
    recursively_build_mid_kd_node(v->ptr, depth + 1);
}

inline float KDTree::surface_area(const std::pair<Vec, Vec> pair) {
    return surface_area(pair.first, pair.second);
}

/*
 * INTERSECTS RAY WITH SCENE
 * Using SHA-KD Tree api;
 */
inline bool high_level_intersect(const Ray &r, float &t, int &id, const Scene &s)
{
    return tree->get_intersection(tree->root, r, t, id, 0);
}

/*
 * To judge whether the given segment intersects with the scene.
 */
inline bool high_level_oriented_segment_intersect(const Vec &start, const Vec &target, const Scene &s)
{
    Ray r(start, (target - start).norm());
    int id;
    float dis = (float) (sqrt((target - start).dot(target - start)) + eps);
    return tree->get_intersection(tree->root, r, dis, id, 0);
}

#endif //GALLIFREY_KDTREE_H
