//
// Created by zihao on 2017/3/26.
//

#ifndef GALLIFREY_KDTREE_H
#define GALLIFREY_KDTREE_H

#include <bits/shared_ptr.h>
#include <vector>
#include "aabb.h"

class KDTree {
private:
    bool flag;
    class node
    {
    public:
        node (auto elems): elems(elems) {}
        ~node () {}
        std::vector<std::pair<AABB *, Face *> > elems;
        std::shared_ptr <node> ptl, ptr;
    };
    Scene *s;
    std::shared_ptr <node> root;
    AABB *boxes;
public:
    KDTree(Scene *s): flag(false), s(s) {
        boxes = (AABB *) operator new(sizeof(AABB) * s->size_f);
        // Here s->size_f - 6 means we don't take the bounding into account.
        for (int i = 0; i < s->size_f - 6; ++i)
            new (boxes + i) AABB(s->f_array[i], s);

        std::vector<std::pair<AABB *, Face *> > all_faces;
        // s->size_f - 6: for the same reason above.
        for (int i = 0; i < s->size_f - 6; ++i)
            all_faces.emplace_back(&boxes[i], &s->f_array[i]);
        root = std::make_shared<node> (new node(all_faces));
    }
    ~KDTree() {delete boxes;}

    void naive_build_tree();
    void high_level_build_tree();

    void
};

/*
 * To build the SAH KDTree by using the algorithm that takes O(n^2) time;
 */
void KDTree::naive_build_tree(){
    flag = true;
    // TODO
}
/*
 * To build the SAH KDTree by using the algorithm that takes O(n log^2 n) time;
 */
void KDTree::high_level_build_tree() {
    flag = true;
    // TODO
}

#endif //GALLIFREY_KDTREE_H
