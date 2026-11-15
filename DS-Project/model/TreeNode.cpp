#include "TreeNode.h"

#include "../contracts/Types.h"

bool TreeNodeLinks::attach(TreeNode* parent, TreeNode* child, int side)
{
    if (parent == nullptr || child == nullptr || parent == child || child->parent != nullptr ||
        (side != Side::Left && side != Side::Right))
    {
        return false;
    }
    TreeNode* ancestor = parent;
    while (ancestor != nullptr)
    {
        if (ancestor == child)
        {
            return false;
        }
        ancestor = ancestor->parent;
    }
    if (side == Side::Left)
    {
        if (parent->left != nullptr) return false;
        parent->left = child;
    }
    else
    {
        if (parent->right != nullptr) return false;
        parent->right = child;
    }
    child->parent = parent;
    return true;
}

TreeNode* TreeNodeLinks::detach(TreeNode* parent, int side)
{
    if (parent == nullptr || (side != Side::Left && side != Side::Right))
    {
        return nullptr;
    }
    TreeNode*& slot = side == Side::Left ? parent->left : parent->right;
    TreeNode* child = slot;
    if (child != nullptr)
    {
        slot = nullptr;
        child->parent = nullptr;
    }
    return child;
}
