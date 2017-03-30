//
// Created by zihao on 2017/3/26.
//

#ifndef GALLIFREY_KDTREE_H
#define GALLIFREY_KDTREE_H

#include "aabb.h"

class KDTree {
private:
    struct node
    {
        AABB *box;
        node *lc, *rc;
    };

public:
    KDTree() {
    }
};

#endif //GALLIFREY_KDTREE_H
