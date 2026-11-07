#pragma once
// contracts/ViewSnapshot.h
// Read-only picture of the model for the GUI. No SFML, no pointers into the model.

#include <vector>
#include "Types.h"

// One tree node as the GUI sees it.
struct ViewNode {
    int  id = kNull;   // unique, stable across rotations and swaps
    int  key = 0;       // value shown inside the room
    int  left = kNull;   // child ids, kNull = none
    int  right = kNull;
    int  parent = kNull;
    int  height = 0;       // leaf = 1 (AVL only, else 0)
    int  size = 0;       // subtree size (AVL only, else 0)
    int  balance = 0;       // height(left) - height(right) (AVL only)
    bool alive = true;    // false = deleted slot in the node pool
};

// Heap as the GUI sees it.
struct ViewHeap {
    std::vector<int> array;     // whole array: active heap + sorted region
    int              activeSize = 0;  // array[0..activeSize-1] is the heap,
    // array[activeSize..] is the sorted region
};

// Complete picture. Rebuilt by SnapshotBuilder after every action.
struct ViewSnapshot {
    TreeKind              kind = TreeKind::BST;
    int                   rootId = kNull;
    std::vector<ViewNode> nodes;   // indexed by node id (pool index); check alive
    ViewHeap              heap;    // used only when kind == TreeKind::MaxHeap
};