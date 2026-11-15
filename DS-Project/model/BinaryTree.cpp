#include "BinaryTree.h"
#include "TreeNode.h"

namespace
{
    void recordVisit(TreeNode* node, EventLog* log)
    {
        if (node != nullptr && log != nullptr)
        {
            Event event(EventType::VISIT);
            event.id = node->id;
            emit(log, event);
        }
    }

    bool valuesAreUnique(const IntArray& values)
    {
        for (int i = 0; i < values.size(); i++)
        {
            for (int j = i + 1; j < values.size(); j++)
            {
                if (values[i] == values[j])
                {
                    return false;
                }
            }
        }
        return true;
    }

    int findValue(const IntArray& values, int value, int first, int last)
    {
        for (int i = first; i <= last; i++)
        {
            if (values[i] == value)
            {
                return i;
            }
        }
        return kNull;
    }
}

BinaryTree::BinaryTree() : m_root(nullptr)
{
}

BinaryTree::~BinaryTree()
{
    clear();
}

TreeNode* BinaryTree::root() const
{
    return m_root;
}

int BinaryTree::nodeCount() const
{
    return m_pool.aliveCount();
}

bool BinaryTree::isAlive(int nodeId) const
{
    return m_pool.isAlive(nodeId);
}

const NodePtrArray& BinaryTree::pool() const
{
    return m_pool.pool();
}

bool BinaryTree::owns(const TreeNode* node) const
{
    if (node == nullptr || node->id < 0 || node->id >= m_pool.size())
    {
        return false;
    }
    return m_pool.isAlive(node->id) && m_pool.get(node->id) == node;
}

bool BinaryTree::isReachable(const TreeNode* target) const
{
    if (target == nullptr || m_root == nullptr)
    {
        return false;
    }
    NodeStack pending;
    pending.push(m_root);
    while (!pending.empty())
    {
        TreeNode* current = pending.top();
        pending.pop();
        if (current == target)
        {
            return true;
        }
        if (current->right != nullptr)
        {
            pending.push(current->right);
        }
        if (current->left != nullptr)
        {
            pending.push(current->left);
        }
    }
    return false;
}

TreeNode* BinaryTree::createNode(int value)
{
    return m_pool.createTree(value);
}

bool BinaryTree::setRoot(TreeNode* node, EventLog* log)
{
    if (m_root != nullptr || !owns(node) || node->parent != nullptr || node == m_root)
    {
        return false;
    }
    m_root = node;
    Event inserted(EventType::INSERT);
    inserted.id = node->id;
    inserted.id2 = kNull;
    inserted.side = Side::None;
    inserted.key = node->value;
    emit(log, inserted);
    return true;
}

bool BinaryTree::attachLeft(TreeNode* parent, TreeNode* child, EventLog* log)
{
    return attachChild(parent, child, Side::Left, log);
}

bool BinaryTree::attachRight(TreeNode* parent, TreeNode* child, EventLog* log)
{
    return attachChild(parent, child, Side::Right, log);
}

bool BinaryTree::attachChild(TreeNode* parent, TreeNode* child, int side, EventLog* log)
{
    if (!owns(parent) || !owns(child) || parent == child || child->parent != nullptr ||
        child == m_root || !isReachable(parent))
    {
        return false;
    }
    if ((side == Side::Left && parent->left != nullptr) ||
        (side == Side::Right && parent->right != nullptr) ||
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

    if (!TreeNodeLinks::attach(parent, child, side))
    {
        return false;
    }
    Event inserted(EventType::INSERT);
    inserted.id = child->id;
    inserted.id2 = parent->id;
    inserted.side = side;
    inserted.key = child->value;
    emit(log, inserted);
    return true;
}

bool BinaryTree::validTraversalInputs(const IntArray& order, const IntArray& inorderValues) const
{
    if (order.size() != inorderValues.size() || !valuesAreUnique(order) || !valuesAreUnique(inorderValues))
    {
        return false;
    }
    for (int i = 0; i < order.size(); i++)
    {
        if (inorderValues.indexOf(order[i]) == kNull)
        {
            return false;
        }
    }
    return true;
}

bool BinaryTree::build(const IntArray& preorderValues, const IntArray& inorderValues, EventLog* log)
{
    return buildFromPreIn(preorderValues, inorderValues, log);
}

bool BinaryTree::buildFromPreIn(const IntArray& preorderValues, const IntArray& inorderValues, EventLog* log)
{
    if (!validTraversalInputs(preorderValues, inorderValues))
    {
        return false;
    }
    clear();
    int preorderIndex = 0;
    bool ok = true;
    if (preorderValues.size() > 0)
    {
        buildPreRec(preorderValues, inorderValues, preorderIndex, 0,
                    inorderValues.size() - 1, nullptr, Side::None, log, ok);
    }
    if (!ok || preorderIndex != preorderValues.size())
    {
        clear();
        return false;
    }
    return true;
}

bool BinaryTree::buildFromPostIn(const IntArray& postorderValues, const IntArray& inorderValues, EventLog* log)
{
    if (!validTraversalInputs(postorderValues, inorderValues))
    {
        return false;
    }
    clear();
    int postorderIndex = postorderValues.size() - 1;
    bool ok = true;
    if (postorderValues.size() > 0)
    {
        buildPostRec(postorderValues, inorderValues, postorderIndex, 0,
                     inorderValues.size() - 1, nullptr, Side::None, log, ok);
    }
    if (!ok || postorderIndex != -1)
    {
        clear();
        return false;
    }
    return true;
}

TreeNode* BinaryTree::buildPreRec(const IntArray& preorderValues, const IntArray& inorderValues,
                                  int& preorderIndex, int inLeft, int inRight,
                                  TreeNode* parent, int side, EventLog* log, bool& ok)
{
    if (!ok || inLeft > inRight)
    {
        return nullptr;
    }
    if (preorderIndex >= preorderValues.size())
    {
        ok = false;
        return nullptr;
    }

    const int value = preorderValues[preorderIndex];
    preorderIndex = preorderIndex + 1;
    const int split = findValue(inorderValues, value, inLeft, inRight);
    if (split == kNull)
    {
        ok = false;
        return nullptr;
    }

    TreeNode* node = m_pool.createTree(value);
    node->parent = parent;
    if (parent == nullptr)
    {
        m_root = node;
    }
    else if (side == Side::Left)
    {
        parent->left = node;
    }
    else
    {
        parent->right = node;
    }
    Event inserted(EventType::INSERT);
    inserted.id = node->id;
    inserted.id2 = parent == nullptr ? kNull : parent->id;
    inserted.side = side;
    inserted.key = value;
    emit(log, inserted);

    node->left = buildPreRec(preorderValues, inorderValues, preorderIndex,
                             inLeft, split - 1, node, Side::Left, log, ok);
    node->right = buildPreRec(preorderValues, inorderValues, preorderIndex,
                              split + 1, inRight, node, Side::Right, log, ok);
    return node;
}

TreeNode* BinaryTree::buildPostRec(const IntArray& postorderValues, const IntArray& inorderValues,
                                   int& postorderIndex, int inLeft, int inRight,
                                   TreeNode* parent, int side, EventLog* log, bool& ok)
{
    if (!ok || inLeft > inRight)
    {
        return nullptr;
    }
    if (postorderIndex < 0)
    {
        ok = false;
        return nullptr;
    }

    const int value = postorderValues[postorderIndex];
    postorderIndex = postorderIndex - 1;
    const int split = findValue(inorderValues, value, inLeft, inRight);
    if (split == kNull)
    {
        ok = false;
        return nullptr;
    }

    TreeNode* node = m_pool.createTree(value);
    node->parent = parent;
    if (parent == nullptr)
    {
        m_root = node;
    }
    else if (side == Side::Left)
    {
        parent->left = node;
    }
    else
    {
        parent->right = node;
    }
    Event inserted(EventType::INSERT);
    inserted.id = node->id;
    inserted.id2 = parent == nullptr ? kNull : parent->id;
    inserted.side = side;
    inserted.key = value;
    emit(log, inserted);

    node->right = buildPostRec(postorderValues, inorderValues, postorderIndex,
                               split + 1, inRight, node, Side::Right, log, ok);
    node->left = buildPostRec(postorderValues, inorderValues, postorderIndex,
                              inLeft, split - 1, node, Side::Left, log, ok);
    return node;
}

void BinaryTree::preorderRec(TreeNode* node, IntArray& out, EventLog* log) const
{
    if (node == nullptr)
    {
        return;
    }
    recordVisit(node, log);
    out.push(node->id);
    preorderRec(node->left, out, log);
    preorderRec(node->right, out, log);
}

void BinaryTree::inorderRec(TreeNode* node, IntArray& out, EventLog* log) const
{
    if (node == nullptr)
    {
        return;
    }
    inorderRec(node->left, out, log);
    recordVisit(node, log);
    out.push(node->id);
    inorderRec(node->right, out, log);
}

void BinaryTree::postorderRec(TreeNode* node, IntArray& out, EventLog* log) const
{
    if (node == nullptr)
    {
        return;
    }
    postorderRec(node->left, out, log);
    postorderRec(node->right, out, log);
    recordVisit(node, log);
    out.push(node->id);
}

void BinaryTree::levelAtDepth(TreeNode* node, int depth, int targetDepth, IntArray& out, EventLog* log) const
{
    if (node == nullptr)
    {
        return;
    }
    if (depth == targetDepth)
    {
        recordVisit(node, log);
        out.push(node->id);
        return;
    }
    levelAtDepth(node->left, depth + 1, targetDepth, out, log);
    levelAtDepth(node->right, depth + 1, targetDepth, out, log);
}

IntArray BinaryTree::preorderRecursive(EventLog* log) const
{
    IntArray out;
    preorderRec(m_root, out, log);
    return out;
}

IntArray BinaryTree::preorderIterative(EventLog* log) const
{
    IntArray out;
    NodeStack pending;
    if (m_root != nullptr)
    {
        pending.push(m_root);
    }
    while (!pending.empty())
    {
        TreeNode* node = pending.top();
        pending.pop();
        recordVisit(node, log);
        out.push(node->id);
        if (node->right != nullptr)
        {
            pending.push(node->right);
        }
        if (node->left != nullptr)
        {
            pending.push(node->left);
        }
    }
    return out;
}

IntArray BinaryTree::inorderRecursive(EventLog* log) const
{
    IntArray out;
    inorderRec(m_root, out, log);
    return out;
}

IntArray BinaryTree::inorderIterative(EventLog* log) const
{
    IntArray out;
    NodeStack pending;
    TreeNode* current = m_root;
    while (current != nullptr || !pending.empty())
    {
        while (current != nullptr)
        {
            pending.push(current);
            current = current->left;
        }
        current = pending.top();
        pending.pop();
        recordVisit(current, log);
        out.push(current->id);
        current = current->right;
    }
    return out;
}

IntArray BinaryTree::postorderRecursive(EventLog* log) const
{
    IntArray out;
    postorderRec(m_root, out, log);
    return out;
}

IntArray BinaryTree::postorderIterative(EventLog* log) const
{
    IntArray out;
    NodeStack pending;
    TreeNode* current = m_root;
    TreeNode* lastVisited = nullptr;
    while (current != nullptr || !pending.empty())
    {
        if (current != nullptr)
        {
            pending.push(current);
            current = current->left;
        }
        else
        {
            TreeNode* candidate = pending.top();
            if (candidate->right != nullptr && lastVisited != candidate->right)
            {
                current = candidate->right;
            }
            else
            {
                recordVisit(candidate, log);
                out.push(candidate->id);
                lastVisited = candidate;
                pending.pop();
            }
        }
    }
    return out;
}

IntArray BinaryTree::levelOrderRecursive(EventLog* log) const
{
    IntArray out;
    const int treeHeight = height(m_root);
    for (int depth = 0; depth < treeHeight; depth++)
    {
        levelAtDepth(m_root, 0, depth, out, log);
    }
    return out;
}

IntArray BinaryTree::levelOrderIterative(EventLog* log) const
{
    IntArray out;
    NodeQueue pending;
    if (m_root != nullptr)
    {
        pending.push(m_root);
    }
    while (!pending.empty())
    {
        TreeNode* node = pending.front();
        pending.pop();
        recordVisit(node, log);
        out.push(node->id);
        if (node->left != nullptr)
        {
            pending.push(node->left);
        }
        if (node->right != nullptr)
        {
            pending.push(node->right);
        }
    }
    return out;
}

IntArray BinaryTree::preorder(EventLog* log) const
{
    return preorderIterative(log);
}

IntArray BinaryTree::inorder(EventLog* log) const
{
    return inorderIterative(log);
}

IntArray BinaryTree::postorder(EventLog* log) const
{
    return postorderIterative(log);
}

IntArray BinaryTree::levelOrder(EventLog* log) const
{
    return levelOrderIterative(log);
}

int BinaryTree::height(const TreeNode* node)
{
    if (node == nullptr)
    {
        return kEmptyHeight;
    }
    const int leftHeight = height(node->left);
    const int rightHeight = height(node->right);
    return (leftHeight > rightHeight ? leftHeight : rightHeight) + 1;
}

int BinaryTree::height() const
{
    return height(m_root);
}

int BinaryTree::diameterHeight(TreeNode* node, int& bestEdges) const
{
    if (node == nullptr)
    {
        return 0;
    }
    const int leftHeight = diameterHeight(node->left, bestEdges);
    const int rightHeight = diameterHeight(node->right, bestEdges);
    const int throughNode = leftHeight + rightHeight;
    if (throughNode > bestEdges)
    {
        bestEdges = throughNode;
    }
    return (leftHeight > rightHeight ? leftHeight : rightHeight) + 1;
}

int BinaryTree::diameter() const
{
    int bestEdges = 0;
    diameterHeight(m_root, bestEdges);
    return bestEdges;
}

TreeNode* BinaryTree::lcaRec(TreeNode* node, int nodeIdA, int nodeIdB, EventLog* log) const
{
    if (node == nullptr)
    {
        return nullptr;
    }
    recordVisit(node, log);
    if (node->id == nodeIdA || node->id == nodeIdB)
    {
        return node;
    }
    TreeNode* left = lcaRec(node->left, nodeIdA, nodeIdB, log);
    TreeNode* right = lcaRec(node->right, nodeIdA, nodeIdB, log);
    if (left != nullptr && right != nullptr)
    {
        return node;
    }
    return left != nullptr ? left : right;
}

int BinaryTree::lca(int nodeIdA, int nodeIdB, EventLog* log) const
{
    if (nodeIdA < 0 || nodeIdB < 0 || nodeIdA >= m_pool.size() || nodeIdB >= m_pool.size() ||
        !m_pool.isAlive(nodeIdA) || !m_pool.isAlive(nodeIdB))
    {
        return kNull;
    }
    TreeNode* common = lcaRec(m_root, nodeIdA, nodeIdB, log);
    if (common == nullptr)
    {
        return kNull;
    }
    if (nodeIdA != nodeIdB)
    {
        NodeStack pending;
        pending.push(m_root);
        bool foundA = false;
        bool foundB = false;
        while (!pending.empty())
        {
            TreeNode* node = pending.top();
            pending.pop();
            foundA = foundA || node->id == nodeIdA;
            foundB = foundB || node->id == nodeIdB;
            if (node->left != nullptr)
            {
                pending.push(node->left);
            }
            if (node->right != nullptr)
            {
                pending.push(node->right);
            }
        }
        if (!foundA || !foundB)
        {
            return kNull;
        }
    }
    return common->id;
}

bool BinaryTree::pathRec(TreeNode* node, int targetId, IntArray& outIds, EventLog* log) const
{
    if (node == nullptr)
    {
        return false;
    }
    recordVisit(node, log);
    outIds.push(node->id);
    if (node->id == targetId || pathRec(node->left, targetId, outIds, log) ||
        pathRec(node->right, targetId, outIds, log))
    {
        return true;
    }
    outIds.pop();
    return false;
}

bool BinaryTree::pathTo(int nodeId, IntArray& outIds, EventLog* log) const
{
    outIds.clear();
    if (nodeId < 0 || nodeId >= m_pool.size() || !m_pool.isAlive(nodeId))
    {
        return false;
    }
    if (!pathRec(m_root, nodeId, outIds, log))
    {
        outIds.clear();
        return false;
    }
    return true;
}

IntArray BinaryTree::pathTo(int nodeId, EventLog* log) const
{
    IntArray out;
    pathTo(nodeId, out, log);
    return out;
}

void BinaryTree::mirrorRec(TreeNode* node, EventLog* log)
{
    if (node == nullptr)
    {
        return;
    }
    recordVisit(node, log);
    TreeNode* temporary = node->left;
    node->left = node->right;
    node->right = temporary;
    mirrorRec(node->left, log);
    mirrorRec(node->right, log);
}

void BinaryTree::mirror(EventLog* log)
{
    mirrorRec(m_root, log);
}

bool BinaryTree::sameRec(const TreeNode* a, const TreeNode* b) const
{
    if (a == nullptr || b == nullptr)
    {
        return a == nullptr && b == nullptr;
    }
    return a->value == b->value && sameRec(a->left, b->left) && sameRec(a->right, b->right);
}

bool BinaryTree::isSameTree(const BinaryTree& other) const
{
    return sameRec(m_root, other.m_root);
}

bool BinaryTree::symmetricRec(const TreeNode* left, const TreeNode* right) const
{
    if (left == nullptr || right == nullptr)
    {
        return left == nullptr && right == nullptr;
    }
    return left->value == right->value &&
           symmetricRec(left->left, right->right) && symmetricRec(left->right, right->left);
}

bool BinaryTree::isSymmetric() const
{
    return m_root == nullptr || symmetricRec(m_root->left, m_root->right);
}

void BinaryTree::clear()
{
    m_root = nullptr;
    m_pool.clear();
}
