#include "BST.h"
#include "TreeNode.h"

BST::BST() : m_root(nullptr)
{
}

BST::~BST()
{
    clear();
}

BSTNode* BST::root() const
{
    return m_root;
}

int BST::nodeCount() const
{
    return m_pool.aliveCount();
}

bool BST::isAlive(int nodeId) const
{
    return m_pool.isAlive(nodeId);
}

const NodePtrArray& BST::pool() const
{
    return m_pool.pool();
}

bool BST::owns(const BSTNode* node) const
{
    return node != nullptr && node->id >= 0 && node->id < m_pool.size() &&
           m_pool.isAlive(node->id) && m_pool.get(node->id) == node;
}

BSTNode* BST::findWithoutEvents(int key) const
{
    BSTNode* current = m_root;
    while (current != nullptr && current->key != key)
    {
        current = key < current->key ? static_cast<BSTNode*>(current->left)
                                     : static_cast<BSTNode*>(current->right);
    }
    return current;
}

BSTNode* BST::search(int key, EventLog* log) const
{
    BSTNode* current = m_root;
    while (current != nullptr)
    {
        Event event(EventType::COMPARE);
        event.id = current->id;
        event.key = key;
        event.result = key == current->key ? CompareResult::Equal : (key < current->key ? CompareResult::Less : CompareResult::Greater);
        event.alarmCost = 1;
        emit(log, event);
        if (key == current->key)
        {
            return current;
        }
        current = key < current->key ? static_cast<BSTNode*>(current->left)
                                     : static_cast<BSTNode*>(current->right);
    }
    return nullptr;
}

bool BST::insert(int key, EventLog* log)
{
    BSTNode* parent = nullptr;
    BSTNode* current = m_root;
    int side = Side::None;
    while (current != nullptr)
    {
        parent = current;
        Event compare(EventType::COMPARE);
        compare.id = current->id;
        compare.key = key;
        compare.result = key == current->key ? CompareResult::Equal : (key < current->key ? CompareResult::Less : CompareResult::Greater);
        compare.alarmCost = 1;
        emit(log, compare);
        if (key == current->key)
        {
            return false;
        }
        side = key < current->key ? Side::Left : Side::Right;
        current = side == Side::Left ? static_cast<BSTNode*>(current->left)
                                     : static_cast<BSTNode*>(current->right);
    }

    BSTNode* node = m_pool.createBST(key);
    if (parent == nullptr)
    {
        m_root = node;
    }
    else if (!TreeNodeLinks::attach(parent, node, side))
    {
        m_pool.markDead(node->id);
        return false;
    }
    Event inserted(EventType::INSERT);
    inserted.id = node->id;
    inserted.id2 = parent == nullptr ? kNull : parent->id;
    inserted.side = side;
    inserted.key = key;
    emit(log, inserted);
    return true;
}

bool BST::remove(int key, EventLog* log)
{
    BSTNode* node = search(key, log);
    if (node == nullptr)
    {
        return false;
    }

    if (node->left != nullptr && node->right != nullptr)
    {
        BSTNode* next = static_cast<BSTNode*>(node->right);
        while (next->left != nullptr)
        {
            next = static_cast<BSTNode*>(next->left);
        }
        const int oldValue = node->key;
        const int nextValue = next->key;
        node->key = nextValue;
        node->treasure = next->treasure;
        node->value = nextValue;
        next->key = oldValue;
        next->treasure = oldValue;
        next->value = oldValue;
        Event swapped(EventType::SWAP_VALUE);
        swapped.id = node->id;
        swapped.id2 = next->id;
        swapped.oldV = oldValue;
        swapped.newV = nextValue;
        emit(log, swapped);
        node = next;
    }

    BSTNode* child = node->left != nullptr ? static_cast<BSTNode*>(node->left)
                                          : static_cast<BSTNode*>(node->right);
    BSTNode* parent = static_cast<BSTNode*>(node->parent);
    const int side = parent == nullptr ? Side::None : (parent->left == node ? Side::Left : Side::Right);
    if (parent == nullptr)
    {
        m_root = child;
    }
    else if (side == Side::Left)
    {
        parent->left = child;
    }
    else
    {
        parent->right = child;
    }
    if (child != nullptr)
    {
        child->parent = parent;
    }
    node->parent = nullptr;
    node->left = nullptr;
    node->right = nullptr;
    m_pool.markDead(node->id);
    Event unlinked(EventType::UNLINK);
    unlinked.id = node->id;
    unlinked.id2 = parent == nullptr ? kNull : parent->id;
    unlinked.side = side;
    unlinked.key = node->key;
    emit(log, unlinked);
    return true;
}

BSTNode* BST::findMin() const
{
    BSTNode* node = m_root;
    while (node != nullptr && node->left != nullptr)
    {
        node = static_cast<BSTNode*>(node->left);
    }
    return node;
}

BSTNode* BST::findMax() const
{
    BSTNode* node = m_root;
    while (node != nullptr && node->right != nullptr)
    {
        node = static_cast<BSTNode*>(node->right);
    }
    return node;
}

BSTNode* BST::successor(int key) const
{
    BSTNode* node = findWithoutEvents(key);
    if (node == nullptr)
    {
        return nullptr;
    }
    if (node->right != nullptr)
    {
        node = static_cast<BSTNode*>(node->right);
        while (node->left != nullptr)
        {
            node = static_cast<BSTNode*>(node->left);
        }
        return node;
    }
    BSTNode* parent = static_cast<BSTNode*>(node->parent);
    while (parent != nullptr && parent->right == node)
    {
        node = parent;
        parent = static_cast<BSTNode*>(parent->parent);
    }
    return parent;
}

BSTNode* BST::predecessor(int key) const
{
    BSTNode* node = findWithoutEvents(key);
    if (node == nullptr)
    {
        return nullptr;
    }
    if (node->left != nullptr)
    {
        node = static_cast<BSTNode*>(node->left);
        while (node->right != nullptr)
        {
            node = static_cast<BSTNode*>(node->right);
        }
        return node;
    }
    BSTNode* parent = static_cast<BSTNode*>(node->parent);
    while (parent != nullptr && parent->left == node)
    {
        node = parent;
        parent = static_cast<BSTNode*>(parent->parent);
    }
    return parent;
}

void BST::rangeRec(BSTNode* node, int low, int high, IntArray& out, EventLog* log) const
{
    if (node == nullptr)
    {
        return;
    }

    Event visit(EventType::VISIT);
    visit.id = node->id;
    visit.alarmCost = 1;
    emit(log, visit);

    if (node->key >= low && node->key <= high)
    {
        out.push(node->key);
    }

    // Left subtree holds keys below node->key. Skip it when every such key is below low.
    if (node->key > low)
    {
        rangeRec(static_cast<BSTNode*>(node->left), low, high, out, log);
    }
    else if (node->left != nullptr)
    {
        emitSkipped(static_cast<BSTNode*>(node->left), log);
    }

    // Right subtree holds keys above node->key. Skip it when every such key is above high.
    if (node->key < high)
    {
        rangeRec(static_cast<BSTNode*>(node->right), low, high, out, log);
    }
    else if (node->right != nullptr)
    {
        emitSkipped(static_cast<BSTNode*>(node->right), log);
    }
}

void BST::emitSkipped(BSTNode* subtreeRoot, EventLog* log) const
{
    Event skip(EventType::HIGHLIGHT_SUBTREE);
    skip.id = subtreeRoot->id;
    skip.result = HighlightMode::Skipped;
    emit(log, skip);
}

void BST::rangeQuery(int low, int high, IntArray& out, EventLog* log) const
{
    out.clear();
    if (low <= high)
    {
        rangeRec(m_root, low, high, out, log);
    }
}

IntArray BST::rangeQuery(int low, int high, EventLog* log) const
{
    IntArray out;
    rangeQuery(low, high, out, log);
    return out;
}

int BST::kthSmallest(int k) const
{
    if (k < 1 || k > nodeCount())
    {
        return kNull;
    }
    NodeStack pending;
    BSTNode* current = m_root;
    int index = 0;
    while (current != nullptr || !pending.empty())
    {
        while (current != nullptr)
        {
            pending.push(current);
            current = static_cast<BSTNode*>(current->left);
        }
        current = static_cast<BSTNode*>(pending.top());
        pending.pop();
        index = index + 1;
        if (index == k)
        {
            return current->key;
        }
        current = static_cast<BSTNode*>(current->right);
    }
    return kNull;
}

void BST::clear()
{
    m_root = nullptr;
    m_pool.clear();
}
