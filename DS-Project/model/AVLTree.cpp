#include "AVLTree.h"

#include "../contracts/Util.h"

AVLTree::AVLTree() : m_root(nullptr)
{
}

AVLTree::~AVLTree()
{
    clear();
}

AVLNode* AVLTree::root() const
{
    return m_root;
}

int AVLTree::nodeCount() const
{
    return m_pool.aliveCount();
}

bool AVLTree::isAlive(int nodeId) const
{
    return m_pool.isAlive(nodeId);
}

const NodePtrArray& AVLTree::pool() const
{
    return m_pool.pool();
}

bool AVLTree::owns(const AVLNode* node) const
{
    if (node == nullptr || node->id < 0 || node->id >= m_pool.size())
    {
        return false;
    }
    return m_pool.isAlive(node->id) && m_pool.get(node->id) == node;
}

int AVLTree::height(const AVLNode* node)
{
    return node == nullptr ? kEmptyHeight : node->height;
}

int AVLTree::balanceFactor(const AVLNode* node)
{
    if (node == nullptr)
    {
        return 0;
    }
    return height(node->left == nullptr ? nullptr : static_cast<const AVLNode*>(node->left)) -
           height(node->right == nullptr ? nullptr : static_cast<const AVLNode*>(node->right));
}

int AVLTree::height() const
{
    return height(m_root);
}

AVLNode* AVLTree::findWithoutEvents(int key) const
{
    AVLNode* current = m_root;
    while (current != nullptr)
    {
        if (key == current->key)
        {
            return current;
        }
        current = key < current->key
            ? static_cast<AVLNode*>(current->left)
            : static_cast<AVLNode*>(current->right);
    }
    return nullptr;
}

AVLNode* AVLTree::find(int key, EventLog* log) const
{
    AVLNode* current = m_root;
    while (current != nullptr)
    {
        Event event(EventType::COMPARE);
        event.id = current->id;
        event.key = key;
        if (key < current->key)
        {
            event.result = CompareResult::Less;
            emit(log, event);
            current = static_cast<AVLNode*>(current->left);
        }
        else if (key > current->key)
        {
            event.result = CompareResult::Greater;
            emit(log, event);
            current = static_cast<AVLNode*>(current->right);
        }
        else
        {
            event.result = CompareResult::Equal;
            emit(log, event);
            return current;
        }
    }
    return nullptr;
}

bool AVLTree::contains(int key) const
{
    return findWithoutEvents(key) != nullptr;
}

void AVLTree::updateMetadata(AVLNode* node, EventLog* log)
{
    if (node == nullptr)
    {
        return;
    }

    const int leftHeight = height(node->left == nullptr ? nullptr : static_cast<AVLNode*>(node->left));
    const int rightHeight = height(node->right == nullptr ? nullptr : static_cast<AVLNode*>(node->right));
    const int newHeight = maxI(leftHeight, rightHeight) + 1;
    if (node->height != newHeight)
    {
        Event event(EventType::UPDATE_HEIGHT);
        event.id = node->id;
        event.oldV = node->height;
        event.newV = newHeight;
        event.undo[0] = node->height;
        emit(log, event);
        node->height = newHeight;
    }

    const int leftSize = node->left == nullptr ? 0 : static_cast<AVLNode*>(node->left)->size;
    const int rightSize = node->right == nullptr ? 0 : static_cast<AVLNode*>(node->right)->size;
    const int newSize = leftSize + rightSize + 1;
    if (node->size != newSize)
    {
        Event event(EventType::UPDATE_SIZE);
        event.id = node->id;
        event.oldV = node->size;
        event.newV = newSize;
        event.undo[0] = node->size;
        emit(log, event);
        node->size = newSize;
    }
}

AVLNode* AVLTree::rotateLeftRaw(AVLNode* pivot, EventLog* log)
{
    if (pivot == nullptr || pivot->right == nullptr)
    {
        return pivot;
    }

    AVLNode* raised = static_cast<AVLNode*>(pivot->right);
    TreeNode* middle = raised->left;
    TreeNode* parent = pivot->parent;
    int parentSide = Side::None;
    if (parent != nullptr)
    {
        parentSide = parent->left == pivot ? Side::Left : Side::Right;
    }

    raised->parent = parent;
    if (parent == nullptr)
    {
        m_root = raised;
    }
    else if (parentSide == Side::Left)
    {
        parent->left = raised;
    }
    else
    {
        parent->right = raised;
    }

    raised->left = pivot;
    pivot->parent = raised;
    pivot->right = middle;
    if (middle != nullptr)
    {
        middle->parent = pivot;
    }

    updateMetadata(pivot, log);
    updateMetadata(raised, log);
    return raised;
}

AVLNode* AVLTree::rotateRightRaw(AVLNode* pivot, EventLog* log)
{
    if (pivot == nullptr || pivot->left == nullptr)
    {
        return pivot;
    }

    AVLNode* raised = static_cast<AVLNode*>(pivot->left);
    TreeNode* middle = raised->right;
    TreeNode* parent = pivot->parent;
    int parentSide = Side::None;
    if (parent != nullptr)
    {
        parentSide = parent->left == pivot ? Side::Left : Side::Right;
    }

    raised->parent = parent;
    if (parent == nullptr)
    {
        m_root = raised;
    }
    else if (parentSide == Side::Left)
    {
        parent->left = raised;
    }
    else
    {
        parent->right = raised;
    }

    raised->right = pivot;
    pivot->parent = raised;
    pivot->left = middle;
    if (middle != nullptr)
    {
        middle->parent = pivot;
    }

    updateMetadata(pivot, log);
    updateMetadata(raised, log);
    return raised;
}

bool AVLTree::rotateLeft(AVLNode* pivot, EventLog* log)
{
    if (!owns(pivot) || pivot->right == nullptr)
    {
        return false;
    }
    Event event(EventType::ROTATE);
    event.id = pivot->id;
    event.result = RotationType::RR;
    event.undo[0] = pivot->parent == nullptr ? kNull : pivot->parent->id;
    event.undo[1] = pivot->right->id;
    emit(log, event);
    AVLNode* raised = rotateLeftRaw(pivot, log);
    rebalanceFrom(static_cast<AVLNode*>(raised->parent), log);
    return true;
}

bool AVLTree::rotateRight(AVLNode* pivot, EventLog* log)
{
    if (!owns(pivot) || pivot->left == nullptr)
    {
        return false;
    }
    Event event(EventType::ROTATE);
    event.id = pivot->id;
    event.result = RotationType::LL;
    event.undo[0] = pivot->parent == nullptr ? kNull : pivot->parent->id;
    event.undo[1] = pivot->left->id;
    emit(log, event);
    AVLNode* raised = rotateRightRaw(pivot, log);
    rebalanceFrom(static_cast<AVLNode*>(raised->parent), log);
    return true;
}

void AVLTree::rebalanceFrom(AVLNode* node, EventLog* log)
{
    AVLNode* current = node;
    while (current != nullptr)
    {
        updateMetadata(current, log);
        const int factor = balanceFactor(current);
        if (factor > 1)
        {
            const int rotation = balanceFactor(static_cast<AVLNode*>(current->left)) >= 0
                ? RotationType::LL : RotationType::LR;
            Event event(EventType::ROTATE);
            event.id = current->id;
            event.result = rotation;
            emit(log, event);

            AVLNode* subtreeRoot = current;
            if (rotation == RotationType::LR)
            {
                rotateLeftRaw(static_cast<AVLNode*>(current->left), log);
            }
            subtreeRoot = rotateRightRaw(current, log);
            current = static_cast<AVLNode*>(subtreeRoot->parent);
        }
        else if (factor < -1)
        {
            const int rotation = balanceFactor(static_cast<AVLNode*>(current->right)) <= 0
                ? RotationType::RR : RotationType::RL;
            Event event(EventType::ROTATE);
            event.id = current->id;
            event.result = rotation;
            emit(log, event);

            AVLNode* subtreeRoot = current;
            if (rotation == RotationType::RL)
            {
                rotateRightRaw(static_cast<AVLNode*>(current->right), log);
            }
            subtreeRoot = rotateLeftRaw(current, log);
            current = static_cast<AVLNode*>(subtreeRoot->parent);
        }
        else
        {
            current = static_cast<AVLNode*>(current->parent);
        }
    }
}

bool AVLTree::insert(int key, EventLog* log)
{
    AVLNode* parent = nullptr;
    AVLNode* current = m_root;
    int side = Side::None;
    while (current != nullptr)
    {
        parent = current;
        Event compare(EventType::COMPARE);
        compare.id = current->id;
        compare.key = key;
        if (key < current->key)
        {
            compare.result = CompareResult::Less;
            side = Side::Left;
            emit(log, compare);
            current = static_cast<AVLNode*>(current->left);
        }
        else if (key > current->key)
        {
            compare.result = CompareResult::Greater;
            side = Side::Right;
            emit(log, compare);
            current = static_cast<AVLNode*>(current->right);
        }
        else
        {
            compare.result = CompareResult::Equal;
            emit(log, compare);
            return false;
        }
    }

    AVLNode* added = m_pool.createAVL(key);
    if (parent == nullptr)
    {
        m_root = added;
    }
    else
    {
        added->parent = parent;
        if (side == Side::Left)
        {
            parent->left = added;
        }
        else
        {
            parent->right = added;
        }
    }

    Event inserted(EventType::INSERT);
    inserted.id = added->id;
    inserted.id2 = parent == nullptr ? kNull : parent->id;
    inserted.side = side;
    inserted.key = key;
    inserted.undo[0] = parent == nullptr ? kNull : parent->id;
    inserted.undo[1] = side;
    emit(log, inserted);
    rebalanceFrom(parent, log);
    return true;
}

bool AVLTree::remove(int key, EventLog* log)
{
    AVLNode* target = find(key, log);
    if (target == nullptr)
    {
        return false;
    }

    if (target->left != nullptr && target->right != nullptr)
    {
        AVLNode* successor = static_cast<AVLNode*>(target->right);
        while (successor->left != nullptr)
        {
            successor = static_cast<AVLNode*>(successor->left);
        }

        Event swap(EventType::SWAP_VALUE);
        swap.id = target->id;
        swap.id2 = successor->id;
        swap.oldV = target->key;
        swap.newV = successor->key;
        swap.undo[0] = target->treasure;
        swap.undo[1] = successor->treasure;
        emit(log, swap);

        const int oldKey = target->key;
        const int oldValue = target->value;
        const int oldTreasure = target->treasure;
        target->key = successor->key;
        target->value = successor->value;
        target->treasure = successor->treasure;
        successor->key = oldKey;
        successor->value = oldValue;
        successor->treasure = oldTreasure;
        target = successor;
    }

    TreeNode* parent = target->parent;
    TreeNode* child = target->left != nullptr ? target->left : target->right;
    int side = Side::None;
    if (parent != nullptr)
    {
        side = parent->left == target ? Side::Left : Side::Right;
        if (side == Side::Left)
        {
            parent->left = child;
        }
        else
        {
            parent->right = child;
        }
    }
    else
    {
        m_root = child == nullptr ? nullptr : static_cast<AVLNode*>(child);
    }
    if (child != nullptr)
    {
        child->parent = parent;
    }

    Event unlinked(EventType::UNLINK);
    unlinked.id = target->id;
    unlinked.id2 = parent == nullptr ? kNull : parent->id;
    unlinked.side = side;
    unlinked.undo[0] = child == nullptr ? kNull : child->id;
    unlinked.undo[1] = target->left == nullptr ? kNull : target->left->id;
    unlinked.undo[2] = target->right == nullptr ? kNull : target->right->id;
    emit(log, unlinked);

    target->left = nullptr;
    target->right = nullptr;
    target->parent = nullptr;
    m_pool.markDead(target->id);
    rebalanceFrom(parent == nullptr ? static_cast<AVLNode*>(child) : static_cast<AVLNode*>(parent), log);
    return true;
}

bool AVLTree::kth(int k, int& outKey) const
{
    if (k < 1 || k > nodeCount())
    {
        return false;
    }
    AVLNode* current = m_root;
    int remaining = k;
    while (current != nullptr)
    {
        const int leftSize = current->left == nullptr ? 0 : static_cast<AVLNode*>(current->left)->size;
        if (remaining == leftSize + 1)
        {
            outKey = current->key;
            return true;
        }
        if (remaining <= leftSize)
        {
            current = static_cast<AVLNode*>(current->left);
        }
        else
        {
            remaining = remaining - leftSize - 1;
            current = static_cast<AVLNode*>(current->right);
        }
    }
    return false;
}

int AVLTree::kth(int k) const
{
    int key = kNull;
    return kth(k, key) ? key : kNull;
}

int AVLTree::rank(int key) const
{
    AVLNode* current = m_root;
    int count = 0;
    while (current != nullptr)
    {
        if (key < current->key)
        {
            current = static_cast<AVLNode*>(current->left);
        }
        else
        {
            const int leftSize = current->left == nullptr ? 0 : static_cast<AVLNode*>(current->left)->size;
            count = count + leftSize + 1;
            current = key == current->key
                ? nullptr : static_cast<AVLNode*>(current->right);
        }
    }
    return count;
}

int AVLTree::countLessThan(int key) const
{
    AVLNode* current = m_root;
    int count = 0;
    while (current != nullptr)
    {
        if (key <= current->key)
        {
            current = static_cast<AVLNode*>(current->left);
        }
        else
        {
            const int leftSize = current->left == nullptr ? 0 : static_cast<AVLNode*>(current->left)->size;
            count = count + leftSize + 1;
            current = static_cast<AVLNode*>(current->right);
        }
    }
    return count;
}

int AVLTree::countInRange(int lo, int hi) const
{
    if (lo > hi)
    {
        return 0;
    }
    return rank(hi) - countLessThan(lo);
}

bool AVLTree::checkBalanced(const AVLNode* node) const
{
    if (node == nullptr)
    {
        return true;
    }
    const AVLNode* left = node->left == nullptr ? nullptr : static_cast<const AVLNode*>(node->left);
    const AVLNode* right = node->right == nullptr ? nullptr : static_cast<const AVLNode*>(node->right);
    const int leftHeight = height(left);
    const int rightHeight = height(right);
    const int leftSize = left == nullptr ? 0 : left->size;
    const int rightSize = right == nullptr ? 0 : right->size;
    return absI(leftHeight - rightHeight) <= 1 &&
           node->height == maxI(leftHeight, rightHeight) + 1 &&
           node->size == leftSize + rightSize + 1 &&
           checkBalanced(left) && checkBalanced(right);
}

bool AVLTree::isBalanced() const
{
    return checkBalanced(m_root);
}

void AVLTree::clear()
{
    m_root = nullptr;
    m_pool.clear();
}
