// contracts/NodeDefs.h
// Node layouts for all tree types. Raw pointers; the tree's NodePool owns and deletes them.
// Ids are unique, dense (= pool index), and NEVER change on rotation or value swap.
#ifndef VH_NODEDEFS_H
#define VH_NODEDEFS_H

#include "Types.h"

// Act 1 general binary tree node.
struct TreeNode
{
    int id;
    int value;
    TreeNode* left;
    TreeNode* right;
    TreeNode* parent;

    TreeNode(int nodeId, int val) : id(nodeId), value(val), left(nullptr), right(nullptr), parent(nullptr)
    {
    }

    virtual ~TreeNode()
    {
    }
};

// Act 2 BST node. key is the ordering key; treasure is the payload shown in the GUI.
struct BSTNode : public TreeNode
{
    int key;
    int treasure;

    BSTNode(int nodeId, int k) : TreeNode(nodeId, k), key(k), treasure(k)
    {
    }
};

// Act 3 AVL node. height: leaf = 1, empty = 0. size: leaf = 1, empty = 0.
// Both are recomputed on load, never saved.
struct AVLNode : public BSTNode
{
    int height;
    int size;

    AVLNode(int nodeId, int k) : BSTNode(nodeId, k), height(kLeafHeight), size(1)
    {
    }
};

#endif
