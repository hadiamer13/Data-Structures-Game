#ifndef VH_TREENODE_H
#define VH_TREENODE_H

#include "../contracts/NodeDefs.h"

// Shared pointer-link operations for model trees. Ownership and reachability
// are checked by the owning tree before it calls these low-level helpers.
class TreeNodeLinks
{
public:
    static bool attach(TreeNode* parent, TreeNode* child, int side);
    static TreeNode* detach(TreeNode* parent, int side);
};

#endif
