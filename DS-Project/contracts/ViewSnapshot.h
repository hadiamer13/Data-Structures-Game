#pragma once
// contracts/ViewSnapshot.h
// Read-only picture of the model for the GUI. No SFML, no pointers into the model.

#include <vector>
#include "contracts/Types.h"

struct ViewNode {
    int  id = VhConst::Null;   // unique, stable across rotations and swaps
    int  key = 0;
    int  left = VhConst::Null;
    int  right = VhConst::Null;
    int  parent = VhConst::Null;
    int  height = 0;               // leaf = 1 (AVL only, else 0)
    int  size = 0;               // subtree size (AVL only, else 0)
    int  balance = 0;               // height(left) - height(right) (AVL only)
    bool alive = true;            // false = deleted slot in the node pool
};

struct ViewHeap {
    std::vector<int> array;         // active heap + sorted region
    int              activeSize = 0;
};

struct ViewSnapshot {
    int                   kind = TreeKind::BST;
    int                   rootId = VhConst::Null;
    std::vector<ViewNode> nodes;    // indexed by node id; check alive
    ViewHeap              heap;     // used only when kind == TreeKind::MaxHeap
};