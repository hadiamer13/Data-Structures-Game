
#ifndef VH_NODEDEFS_H
#define VH_NODEDEFS_H

#include "Types.h"

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

struct BSTNode : public TreeNode
{
    int key;
    int treasure;

    BSTNode(int nodeId, int k) : TreeNode(nodeId, k), key(k), treasure(k)
    {
    }
};

struct AVLNode : public BSTNode
{
    int height;
    int size;

    AVLNode(int nodeId, int k) : BSTNode(nodeId, k), height(kLeafHeight), size(1)
    {
    }
};

#endif
