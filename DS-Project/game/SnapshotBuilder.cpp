#include "SnapshotBuilder.h"

namespace
{
    int fillNode(const TreeNode* node, ViewSnapshot& view, bool avl)
    {
        if (node == nullptr)
        {
            return 0;
        }

        const int leftHeight = fillNode(node->left, view, avl);
        const int rightHeight = fillNode(node->right, view, avl);
        const int height = (leftHeight > rightHeight ? leftHeight : rightHeight) + 1;
        const int leftSize = node->left == nullptr ? 0 : view.nodes[node->left->id].size;
        const int rightSize = node->right == nullptr ? 0 : view.nodes[node->right->id].size;
        ViewNode& out = view.nodes[node->id];
        out.id = node->id;
        out.key = node->value;

        if (dynamic_cast<const BSTNode*>(node) != nullptr)
        {
            out.key = static_cast<const BSTNode*>(node)->key;
        }

        out.left = node->left == nullptr ? kNull : node->left->id;
        out.right = node->right == nullptr ? kNull : node->right->id;
        out.parent = node->parent == nullptr ? kNull : node->parent->id;
        out.height = avl ? static_cast<const AVLNode*>(node)->height : height;
        out.size = avl ? static_cast<const AVLNode*>(node)->size : leftSize + rightSize + 1;
        out.balance = leftHeight - rightHeight;
        out.alive = true;

        return height;
    }

    void initialize(ViewSnapshot& view, const NodePtrArray& pool, int kind)
    {
        view.kind = kind;
        view.rootId = kNull;
        view.nodes.resize(pool.size());

        for (int i = 0; i < pool.size(); i++)
        {
            view.nodes[i] = ViewNode();
            view.nodes[i].id = i;
        }
    }
}

ViewSnapshot SnapshotBuilder::fromBinaryTree(const BinaryTree& tree)
{
    ViewSnapshot view;
    initialize(view, tree.pool(), TreeKind::Binary);
    view.rootId = tree.root() == nullptr ? kNull : tree.root()->id;
    fillNode(tree.root(), view, false);

    for (int i = 0; i < tree.pool().size(); i++)
    {
        if (!tree.isAlive(i))
        {
            view.nodes[i].alive = false;
        }
    }

    return view;
}

ViewSnapshot SnapshotBuilder::fromBST(const BST& tree)
{
    ViewSnapshot view;
    initialize(view, tree.pool(), TreeKind::BST);
    view.rootId = tree.root() == nullptr ? kNull : tree.root()->id;
    fillNode(tree.root(), view, false);

    for (int i = 0; i < tree.pool().size(); i++)
    {
        if (!tree.isAlive(i))
        {
            view.nodes[i].alive = false;
        }
    }

    return view;
}

ViewSnapshot SnapshotBuilder::fromAVL(const AVLTree& tree)
{
    ViewSnapshot view;
    initialize(view, tree.pool(), TreeKind::AVL);
    view.rootId = tree.root() == nullptr ? kNull : tree.root()->id;
    fillNode(tree.root(), view, true);

    for (int i = 0; i < tree.pool().size(); i++)
    {
        if (!tree.isAlive(i))
        {
            view.nodes[i].alive = false;
        }
    }
    
    return view;
}

ViewSnapshot SnapshotBuilder::fromMaxHeap(const MaxHeap& heap)
{
    ViewSnapshot view;
    view.kind = TreeKind::MaxHeap;
    view.rootId = kNull;
    view.heap.array = heap.array();
    view.heap.activeSize = heap.activeSize();
    return view;
}
