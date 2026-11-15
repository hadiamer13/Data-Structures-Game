#include "Validator.h"

#include "../contracts/Util.h"

namespace
{
    void setError(std::string* error, const char* message)
    {
        if (error != nullptr && error->empty())
        {
            *error = message;
        }
    }

    int inspectBinary(const TreeNode* node, const TreeNode* expectedParent,
                      const NodePtrArray& pool, const BoolArray& alive,
                      BoolArray& visited, std::string* error)
    {
        if (node == nullptr)
        {
            return 0;
        }
        if (node->id < 0 || node->id >= pool.size() || pool[node->id] != node || !alive[node->id])
        {
            setError(error, "Binary tree node id does not match its live pool slot.");
            return -1;
        }
        if (visited[node->id])
        {
            setError(error, "Binary tree contains a cycle or a node with multiple parents.");
            return -1;
        }
        if (node->parent != expectedParent)
        {
            setError(error, "Binary tree parent pointer is inconsistent.");
            return -1;
        }
        visited[node->id] = true;
        const int leftCount = inspectBinary(node->left, node, pool, alive, visited, error);
        const int rightCount = inspectBinary(node->right, node, pool, alive, visited, error);
        if (leftCount < 0 || rightCount < 0)
        {
            return -1;
        }
        return leftCount + rightCount + 1;
    }

    int inspectAVL(const AVLNode* node, const AVLNode* expectedParent,
                   const NodePtrArray& pool, const BoolArray& alive,
                   BoolArray& visited, bool hasLower, int lower,
                   bool hasUpper, int upper, std::string* error,
                   int& computedHeight, int& computedSize)
    {
        if (node == nullptr)
        {
            computedHeight = 0;
            computedSize = 0;
            return 0;
        }
        if (node->id < 0 || node->id >= pool.size() || pool[node->id] != node || !alive[node->id])
        {
            setError(error, "AVL node id does not match its live pool slot.");
            return -1;
        }
        if (visited[node->id])
        {
            setError(error, "AVL tree contains a cycle or a node with multiple parents.");
            return -1;
        }
        if (node->parent != expectedParent)
        {
            setError(error, "AVL parent pointer is inconsistent.");
            return -1;
        }
        if ((hasLower && node->key <= lower) || (hasUpper && node->key >= upper))
        {
            setError(error, "AVL keys violate strict BST ordering.");
            return -1;
        }

        visited[node->id] = true;
        int leftHeight = 0;
        int leftSize = 0;
        int rightHeight = 0;
        int rightSize = 0;
        const AVLNode* left = node->left == nullptr ? nullptr : static_cast<const AVLNode*>(node->left);
        const AVLNode* right = node->right == nullptr ? nullptr : static_cast<const AVLNode*>(node->right);
        const int leftCount = inspectAVL(left, node, pool, alive, visited,
                                         hasLower, lower, true, node->key, error,
                                         leftHeight, leftSize);
        const int rightCount = inspectAVL(right, node, pool, alive, visited,
                                          true, node->key, hasUpper, upper, error,
                                          rightHeight, rightSize);
        if (leftCount < 0 || rightCount < 0)
        {
            return -1;
        }

        computedHeight = maxI(leftHeight, rightHeight) + 1;
        computedSize = leftSize + rightSize + 1;
        if (node->height != computedHeight)
        {
            setError(error, "Stored AVL height differs from the recomputed height.");
            return -1;
        }
        if (node->size != computedSize)
        {
            setError(error, "Stored AVL subtree size differs from the recomputed size.");
            return -1;
        }
        if (absI(leftHeight - rightHeight) > 1)
        {
            setError(error, "AVL balance factor is outside [-1, 1].");
            return -1;
        }
        return leftCount + rightCount + 1;
    }
}

bool Validator::validateBinaryTree(const BinaryTree& tree, std::string* error)
{
    if (error != nullptr)
    {
        error->clear();
    }
    const NodePtrArray& pool = tree.pool();
    BoolArray alive;
    BoolArray visited;
    alive.resize(pool.size());
    visited.resize(pool.size());
    int liveCount = 0;
    for (int i = 0; i < pool.size(); i++)
    {
        alive[i] = tree.isAlive(i);
        if (alive[i])
        {
            liveCount = liveCount + 1;
            if (pool[i]->id != i)
            {
                setError(error, "Binary tree pool ids are not dense.");
                return false;
            }
        }
    }
    const TreeNode* root = tree.root();
    if (root != nullptr && root->parent != nullptr)
    {
        setError(error, "Binary tree root has a parent.");
        return false;
    }
    if (root == nullptr && liveCount != 0)
    {
        setError(error, "Binary tree has live nodes but no root.");
        return false;
    }
    const int visitedCount = inspectBinary(root, nullptr, pool, alive, visited, error);
    if (visitedCount < 0 || visitedCount != liveCount || liveCount != tree.nodeCount())
    {
        setError(error, "Binary tree reachable node count differs from its live node count.");
        return false;
    }
    return true;
}

bool Validator::validateAVL(const AVLTree& tree, std::string* error)
{
    if (error != nullptr)
    {
        error->clear();
    }
    const NodePtrArray& pool = tree.pool();
    BoolArray alive;
    BoolArray visited;
    alive.resize(pool.size());
    visited.resize(pool.size());
    int liveCount = 0;
    for (int i = 0; i < pool.size(); i++)
    {
        alive[i] = tree.isAlive(i);
        if (alive[i])
        {
            liveCount = liveCount + 1;
            if (pool[i]->id != i)
            {
                setError(error, "AVL pool ids are not dense.");
                return false;
            }
        }
    }
    const AVLNode* root = tree.root();
    if (root != nullptr && root->parent != nullptr)
    {
        setError(error, "AVL root has a parent.");
        return false;
    }
    if (root == nullptr && liveCount != 0)
    {
        setError(error, "AVL tree has live nodes but no root.");
        return false;
    }
    int computedHeight = 0;
    int computedSize = 0;
    const int visitedCount = inspectAVL(root, nullptr, pool, alive, visited,
                                        false, 0, false, 0, error,
                                        computedHeight, computedSize);
    if (visitedCount < 0 || visitedCount != liveCount || liveCount != tree.nodeCount())
    {
        setError(error, "AVL reachable node count differs from its live node count.");
        return false;
    }
    return true;
}

bool Validator::validateMaxHeap(const MaxHeap& heap, std::string* error)
{
    if (error != nullptr)
    {
        error->clear();
    }
    if (heap.activeSize() < 0 || heap.activeSize() > heap.totalSize())
    {
        setError(error, "Active heap size is outside the backing array.");
        return false;
    }
    if (!heap.isMaxHeap())
    {
        setError(error, "A parent is smaller than one of its active children.");
        return false;
    }
    if (!heap.isSortedAscending())
    {
        setError(error, "The extracted region is not in ascending order.");
        return false;
    }
    if (heap.activeSize() > 0 && heap.activeSize() < heap.totalSize() &&
        heap.array()[0] > heap.array()[heap.activeSize()])
    {
        setError(error, "The active maximum crosses the sorted-region boundary.");
        return false;
    }
    return true;
}
