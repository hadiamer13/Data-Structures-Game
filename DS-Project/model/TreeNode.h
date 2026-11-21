#ifndef VH_TREENODE_H
#define VH_TREENODE_H

#include "../contracts/NodeDefs.h"

class TreeNodeLinks
{
  public:
    static bool attach(TreeNode* parent, TreeNode* child, int side);
    static TreeNode* detach(TreeNode* parent, int side);
};

#endif
