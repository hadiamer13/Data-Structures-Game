#ifndef VH_SNAPSHOTBUILDER_H
#define VH_SNAPSHOTBUILDER_H

#include "../contracts/ViewSnapshot.h"
#include "../model/BinaryTree.h"
#include "../model/BST.h"
#include "../model/AVLTree.h"
#include "../model/MaxHeap.h"

class SnapshotBuilder
{
  public:
    static ViewSnapshot fromBinaryTree(const BinaryTree& tree);
    static ViewSnapshot fromBST(const BST& tree);
    static ViewSnapshot fromAVL(const AVLTree& tree);
    static ViewSnapshot fromMaxHeap(const MaxHeap& heap);
};

#endif
