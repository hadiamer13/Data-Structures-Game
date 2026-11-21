// game/SandboxSession.cpp
#include "SandboxSession.h"
#include "SnapshotBuilder.h"
#include "../model/Validator.h"
#include "../contracts/Util.h"
#include <cstdint>

namespace
{
    uint32_t nextRandom(uint32_t& s)
    {
        s = s * 1103515245u + 12345u;
        return (s >> 16) & 0x7fffu;
    }

    int heightOf(const TreeNode* n)
    {
        if (n == nullptr)
        {
            return 0;
        }

        return maxI(heightOf(n->left), heightOf(n->right)) + 1;
    }

    std::string joinValues(const IntArray& v)
    {
        std::string s;

        for (int i = 0; i < v.size(); i++)
        {
            if (i == 12)
            {
                s += " ... (" + std::to_string(v.size()) + " total)";
                break;
            }

            if (i > 0)
            {
                s += " ";
            }

            s += std::to_string(v[i]);
        }
        return s;
    }

    int findIdByValue(const BinaryTree& t, int value)
    {
        const NodePtrArray& pool = t.pool();

        for (int i = 0; i < pool.size(); i++)
        {
            if (t.isAlive(i) && pool[i]->value == value)
            {
                return i;
            }
        }

        return kNull;
    }

    void collectRange(const AVLNode* n, int lo, int hi, IntArray& out)
    {
        if (n == nullptr)
        {
            return;
        }

        if (n->key > lo)
        {
            collectRange(static_cast<const AVLNode*>(n->left), lo, hi, out);
        }

        if (n->key >= lo && n->key <= hi)
        {
            out.push(n->key);
        }

        if (n->key < hi)
        {
            collectRange(static_cast<const AVLNode*>(n->right), lo, hi, out);
        }
    }

    std::string num(int v)
    {
        return std::to_string(v);
    }
}

SandboxSession::SandboxSession() : m_kind(TreeKind::BST), m_seed(1), m_empty(false), m_steps(0)
{
    build();
}

bool SandboxSession::isMutating(int op)
{
    return op == SandboxOp::Insert || op == SandboxOp::Delete || op == SandboxOp::RotLL ||
           op == SandboxOp::RotRR || op == SandboxOp::RotLR || op == SandboxOp::RotRL ||
           op == SandboxOp::Mirror || op == SandboxOp::Build || op == SandboxOp::Heapify ||
           op == SandboxOp::SiftUp || op == SandboxOp::Swap || op == SandboxOp::Extract ||
           op == SandboxOp::Sort;
}

int SandboxSession::kind() const
{
    return m_kind;
}

int SandboxSession::steps() const
{
    return m_steps;
}

bool SandboxSession::canUndo() const
{
    return m_history.size() >= 4;
}

void SandboxSession::start(int kind, int seed, bool empty)
{
    m_kind = kind;
    m_seed = seed;
    m_empty = empty;
    m_steps = 0;
    m_history.clear();
    build();
}

void SandboxSession::reset()
{
    m_steps = 0;
    m_history.clear();
    build();
}

void SandboxSession::build()
{
    m_bin.clear();
    m_bst.clear();
    m_avl.clear();
    m_heap.loadState(IntArray(), 0);

    if (m_empty)
    {
        return;
    }

    uint32_t state = (uint32_t)m_seed * 2654435761u + 1u;
    std::string ignoreMsg;
    int ignoreFocus = kNull;

    if (m_kind == TreeKind::MaxHeap)
    {
        IntArray values;

        for (int i = 0; i < 10; i++)
        {
            values.push((int)(nextRandom(state) % 90u) + 10);
        }

        m_heap.buildMaxHeap(values);
        return;
    }

    int placed = 0;
    int attempts = 0;

    while (placed < 7 && attempts < 80)
    {
        attempts++;
        int key = (int)(nextRandom(state) % 90u) + 10;

        if (run(SandboxOp::Insert, key, 0, nullptr, ignoreMsg, ignoreFocus))
        {
            placed++;
        }
    }
}

void SandboxSession::replayHistory()
{
    build();
    std::string msg;
    int focus = kNull;

    for (int i = 0; i + 3 < m_history.size(); i += 4)
    {
        run(m_history[i], m_history[i + 1], m_history[i + 2], nullptr, msg, focus);
    }
}

bool SandboxSession::undo()
{
    if (m_history.size() < 4)
    {
        return false;
    }

    int cost = m_history[m_history.size() - 1];
    m_steps = m_steps - cost;

    for (int i = 0; i < 4; i++)
    {
        m_history.pop();
    }

    replayHistory();
    return true;
}

ViewSnapshot SandboxSession::snapshot() const
{
    if (m_kind == TreeKind::Binary)
    {
        return SnapshotBuilder::fromBinaryTree(m_bin);
    }

    if (m_kind == TreeKind::BST)
    {
        return SnapshotBuilder::fromBST(m_bst);
    }

    if (m_kind == TreeKind::AVL)
    {
        return SnapshotBuilder::fromAVL(m_avl);
    }

    return SnapshotBuilder::fromMaxHeap(m_heap);
}

bool SandboxSession::check(std::string& text) const
{
    std::string err;
    bool ok = true;

    if (m_kind == TreeKind::Binary)
    {
        ok = Validator::validateBinaryTree(m_bin, &err);
    }

    else if (m_kind == TreeKind::BST)
    {
        ok = Validator::validateBST(m_bst, &err);
    }

    else if (m_kind == TreeKind::AVL)
    {
        ok = Validator::validateAVL(m_avl, &err);
    }
    else
    {
        ok = Validator::validateMaxHeap(m_heap, &err);
    }

    text = ok ? "Structure: VALID" : ("Structure: INVALID - " + err);
    return ok;
}

SandboxResult SandboxSession::apply(int op, int a, int b)
{
    SandboxResult r;
    r.before = snapshot();

    EventLog log;
    std::string msg;
    int focus = kNull;
    r.accepted = run(op, a, b, &log, msg, focus);
    r.message = msg;
    r.focusId = focus;
    r.events = log.events();
    r.after = snapshot();
    r.changed = r.accepted && isMutating(op);

    int cost = 0;

    for (int i = 0; i < r.events.size(); i++)
    {
        int t = r.events[i].type;
        if (t == EventType::VISIT || t == EventType::COMPARE)
        {
            cost++;
        }
    }

    m_steps = m_steps + cost;

    if (r.changed)
    {
        m_history.push(op);
        m_history.push(a);
        m_history.push(b);
        m_history.push(cost);
    }

    if (m_kind == TreeKind::MaxHeap && op == SandboxOp::Insert && r.accepted)
    {
        r.before.heap.array.push(a);
        r.before.heap.activeSize = r.before.heap.activeSize + 1;
    }

    r.valid = check(r.validText);
    return r;
}

bool SandboxSession::run(int op, int a, int b, EventLog* log, std::string& msg, int& focus)
{
    focus = kNull;

    if (m_kind == TreeKind::Binary)
    {
        return runBinary(op, a, b, log, msg, focus);
    }

    if (m_kind == TreeKind::BST)
    {
        return runBST(op, a, b, log, msg, focus);
    }

    if (m_kind == TreeKind::AVL)
    {
        return runAVL(op, a, b, log, msg, focus);
    }

    return runHeap(op, a, b, log, msg, focus);
}

// ---------------------------------------------------------------- Binary

bool SandboxSession::runBinary(int op, int a, int b, EventLog* log, std::string& msg, int& focus)
{
    (void)b;
    const NodePtrArray& pool = m_bin.pool();

    if (op == SandboxOp::Insert)
    {
        if (findIdByValue(m_bin, a) != kNull)
        {
            msg = "Value " + num(a) + " is already in the tree (distinct values only).";
            return false;
        }

        TreeNode* node = nullptr;

        if (m_bin.root() == nullptr)
        {
            node = m_bin.createNode(a);
            m_bin.setRoot(node, log);
        }

        else
        {
            IntArray order = m_bin.levelOrder(nullptr);

            for (int i = 0; i < order.size() && node == nullptr; i++)
            {
                TreeNode* p = pool[order[i]];
                if (p->left == nullptr)
                {
                    node = m_bin.createNode(a);
                    m_bin.attachLeft(p, node, log);
                }
                else if (p->right == nullptr)
                {
                    node = m_bin.createNode(a);
                    m_bin.attachRight(p, node, log);
                }
            }
        }

        if (node == nullptr)
        {
            msg = "No free slot found.";
            return false;
        }

        focus = node->id;
        msg = "Inserted " + num(a) + " at the first free slot (level order).";
        return true;
    }

    if (op == SandboxOp::Search)
    {
        IntArray order = m_bin.levelOrder(nullptr);

        for (int i = 0; i < order.size(); i++)
        {
            Event v(EventType::VISIT);
            v.id = order[i];
            emit(log, v);
            if (pool[order[i]]->value == a)
            {
                focus = order[i];
                msg = "Found " + num(a) + " after visiting " + num(i + 1) + " rooms (level order scan).";
                return true;
            }
        }

        msg = num(a) + " is not in the tree (visited all " + num(order.size()) + " rooms).";
        return true;
    }

    if (op == SandboxOp::Preorder || op == SandboxOp::Inorder ||
        op == SandboxOp::Postorder || op == SandboxOp::LevelOrder)
    {
        IntArray ids;
        const char* name = "Preorder";

        if (op == SandboxOp::Preorder)
        {
            ids = m_bin.preorder(log);
        }

        else if (op == SandboxOp::Inorder)
        {
            ids = m_bin.inorder(log);
            name = "Inorder";
        }

        else if (op == SandboxOp::Postorder)
        {
            ids = m_bin.postorder(log);
            name = "Postorder";
        }

        else
        {
            ids = m_bin.levelOrder(log);
            name = "Level order";
        }

        if (ids.size() == 0)
        {
            msg = "The tree is empty.";
            return true;
        }

        IntArray vals;

        for (int i = 0; i < ids.size(); i++)
        {
            vals.push(pool[ids[i]]->value);
        }

        msg = std::string(name) + ": " + joinValues(vals);
        return true;
    }

    if (op == SandboxOp::Mirror)
    {
        if (m_bin.root() == nullptr)
        {
            msg = "The tree is empty.";
            return false;
        }

        m_bin.mirror(log);
        msg = "Mirrored: every room swapped its children.";
        return true;
    }

    if (op == SandboxOp::Info)
    {
        if (m_bin.root() == nullptr)
        {
            msg = "The tree is empty.";
            return true;
        }

        msg = "Rooms " + num(m_bin.nodeCount()) + ", height " + num(m_bin.height()) +
              " (nodes), diameter " + num(m_bin.diameter()) + " (edges), symmetric: " +
              (m_bin.isSymmetric() ? "yes" : "no");

        return true;
    }

    msg = "Not available for a general binary tree.";
    return false;
}

// ------------------------------------------------------------------- BST

bool SandboxSession::runBST(int op, int a, int b, EventLog* log, std::string& msg, int& focus)
{
    if (op == SandboxOp::Insert)
    {
        if (!m_bst.insert(a, log))
        {
            msg = "Duplicate key " + num(a) + " rejected.";
            BSTNode* d = m_bst.search(a, nullptr);
            focus = (d != nullptr) ? d->id : kNull;
            return false;
        }

        BSTNode* n = m_bst.search(a, nullptr);
        focus = (n != nullptr) ? n->id : kNull;
        msg = "Inserted " + num(a) + ".";
        return true;
    }

    if (op == SandboxOp::Search)
    {
        BSTNode* n = m_bst.search(a, log);

        if (n == nullptr)
        {
            msg = num(a) + " is not in the tree.";
            return true;
        }

        focus = n->id;
        msg = "Found " + num(a) + ".";
        return true;
    }

    if (op == SandboxOp::Delete)
    {
        BSTNode* n = m_bst.search(a, nullptr);

        if (n == nullptr)
        {
            msg = "Key " + num(a) + " not found.";
            m_bst.search(a, log); 
            return false;
        }

        const char* kind = (n->left != nullptr && n->right != nullptr) ? "two children (inorder successor swap)"
                                                                       : ((n->left == nullptr && n->right == nullptr) ? "leaf" : "one child (bypass)");
        m_bst.remove(a, log);
        msg = "Deleted " + num(a) + ": " + kind + ".";
        return true;
    }

    if (op == SandboxOp::Range)
    {
        IntArray out;
        m_bst.rangeQuery(a, b, out, log);
        msg = "Keys in [" + num(a) + ", " + num(b) + "]: " + (out.size() == 0 ? std::string("none") : joinValues(out));
        return true;
    }

    if (op == SandboxOp::Kth)
    {
        if (a < 1 || a > m_bst.nodeCount())
        {
            msg = "k must be between 1 and " + num(m_bst.nodeCount()) + " (O(n) inorder scan).";
            return false;
        }

        int key = m_bst.kthSmallest(a);
        BSTNode* n = m_bst.search(key, nullptr);
        focus = (n != nullptr) ? n->id : kNull;
        msg = num(a) + "-th smallest key = " + num(key) + " (O(n) inorder scan).";
        return true;
    }

    if (op == SandboxOp::Min || op == SandboxOp::Max)
    {
        BSTNode* n = (op == SandboxOp::Min) ? m_bst.findMin() : m_bst.findMax();

        if (n == nullptr)
        {
            msg = "The tree is empty.";
            return false;
        }

        focus = n->id;
        msg = std::string(op == SandboxOp::Min ? "Minimum" : "Maximum") + " key = " + num(n->key) + ".";
        return true;
    }

    if (op == SandboxOp::Succ || op == SandboxOp::Pred)
    {
        if (m_bst.search(a, nullptr) == nullptr)
        {
            msg = "Key " + num(a) + " is not in the tree.";
            return false;
        }

        BSTNode* n = (op == SandboxOp::Succ) ? m_bst.successor(a) : m_bst.predecessor(a);

        if (n == nullptr)
        {
            msg = std::string("No ") + (op == SandboxOp::Succ ? "successor" : "predecessor") + " for " + num(a) + ".";
            return true;
        }

        focus = n->id;
        msg = std::string(op == SandboxOp::Succ ? "Successor" : "Predecessor") + " of " + num(a) + " = " + num(n->key) + ".";
        return true;
    }

    if (op == SandboxOp::Info)
    {
        if (m_bst.root() == nullptr)
        {
            msg = "The tree is empty.";
            return true;
        }

        msg = "Keys " + num(m_bst.nodeCount()) + ", height " + num(heightOf(m_bst.root())) +
              ", min " + num(m_bst.findMin()->key) + ", max " + num(m_bst.findMax()->key);

        return true;
    }

    msg = "Not available for a BST.";
    return false;
}

// ------------------------------------------------------------------- AVL

bool SandboxSession::runAVL(int op, int a, int b, EventLog* log, std::string& msg, int& focus)
{
    if (op == SandboxOp::Insert)
    {
        if (!m_avl.insert(a, log))
        {
            msg = "Duplicate key " + num(a) + " rejected.";
            AVLNode* d = m_avl.find(a, nullptr);
            focus = (d != nullptr) ? d->id : kNull;
            return false;
        }

        AVLNode* n = m_avl.find(a, nullptr);
        focus = (n != nullptr) ? n->id : kNull;
        msg = "Inserted " + num(a) + " (rebalanced if needed).";
        return true;
    }

    if (op == SandboxOp::Search)
    {
        AVLNode* n = m_avl.find(a, log);

        if (n == nullptr)
        {
            msg = num(a) + " is not in the tree.";
            return true;
        }

        focus = n->id;
        msg = "Found " + num(a) + ".";
        return true;
    }

    if (op == SandboxOp::Delete)
    {
        if (m_avl.find(a, nullptr) == nullptr)
        {
            msg = "Key " + num(a) + " not found.";
            m_avl.find(a, log);
            return false;
        }

        m_avl.remove(a, log);
        msg = "Deleted " + num(a) + " (rebalanced if needed).";
        return true;
    }

    if (op == SandboxOp::Range)
    {
        IntArray out;
        collectRange(m_avl.root(), a, b, out);
        msg = "Keys in [" + num(a) + ", " + num(b) + "]: " + (out.size() == 0 ? std::string("none") : joinValues(out));
        return true;
    }

    if (op == SandboxOp::Kth)
    {
        int key = kNull;

        if (!m_avl.kth(a, key))
        {
            msg = "k must be between 1 and " + num(m_avl.nodeCount()) + ".";
            return false;
        }

        AVLNode* n = m_avl.find(key, nullptr);
        focus = (n != nullptr) ? n->id : kNull;
        msg = num(a) + "-th smallest key = " + num(key) + " (O(log n) with subtree sizes).";
        return true;
    }

    if (op == SandboxOp::Rank)
    {
        msg = "rank(" + num(a) + ") = " + num(m_avl.rank(a)) + " keys <= " + num(a) + ".";
        return true;
    }

    if (op == SandboxOp::CountRange)
    {
        msg = "countInRange(" + num(a) + ", " + num(b) + ") = " + num(m_avl.countInRange(a, b)) + ".";
        return true;
    }

    if (op == SandboxOp::RotLL || op == SandboxOp::RotRR || op == SandboxOp::RotLR || op == SandboxOp::RotRL)
    {
        if (a < 0 || a >= m_avl.pool().size() || !m_avl.isAlive(a))
        {
            msg = "Click a room first: it is the pivot.";
            return false;
        }

        AVLNode* pv = static_cast<AVLNode*>(m_avl.pool()[a]);
        focus = pv->id;

        if (op == SandboxOp::RotLL)
        {
            if (pv->left == nullptr)
            {
                msg = "LL needs a left child at the pivot.";
                return false;
            }

            m_avl.rotateRight(pv, log);
            msg = "LL: right rotation at the pivot.";
            return true;
        }

        if (op == SandboxOp::RotRR)
        {
            if (pv->right == nullptr)
            {
                msg = "RR needs a right child at the pivot.";
                return false;
            }

            m_avl.rotateLeft(pv, log);
            msg = "RR: left rotation at the pivot.";
            return true;
        }

        if (op == SandboxOp::RotLR)
        {
            AVLNode* child = static_cast<AVLNode*>(pv->left);

            if (child == nullptr || child->right == nullptr)
            {
                msg = "LR needs a left child that has a right child.";
                return false;
            }

            TreeNode* raised = child->right;
            m_avl.rotateLeft(child, log);

            if (pv->parent != raised)
            {
                m_avl.rotateRight(pv, log);
            }

            msg = "LR: left rotation on the child, then right rotation at the pivot.";
            return true;
        }

        AVLNode* child = static_cast<AVLNode*>(pv->right);

        if (child == nullptr || child->left == nullptr)
        {
            msg = "RL needs a right child that has a left child.";
            return false;
        }

        TreeNode* raised = child->left;
        m_avl.rotateRight(child, log);

        if (pv->parent != raised)
        {
            m_avl.rotateLeft(pv, log);
        }

        msg = "RL: right rotation on the child, then left rotation at the pivot.";
        return true;
    }

    if (op == SandboxOp::Info)
    {
        if (m_avl.root() == nullptr)
        {
            msg = "The tree is empty.";
            return true;
        }

        msg = "Keys " + num(m_avl.nodeCount()) + ", height " + num(m_avl.height()) +
              ", balanced: " + (m_avl.isBalanced() ? "yes" : "no");

        return true;
    }

    msg = "Not available for an AVL tree.";
    return false;
}

// ------------------------------------------------------------------ Heap

bool SandboxSession::runHeap(int op, int a, int b, EventLog* log, std::string& msg, int& focus)
{
    (void)focus;
    int active = m_heap.activeSize();
    int total = m_heap.totalSize();

    if (op == SandboxOp::Insert)
    {
        if (active != total)
        {
            msg = "Insert needs an empty sorted region. Use Build or Random first.";
            return false;
        }

        m_heap.insert(a, log);
        msg = "Inserted " + num(a) + " and sifted it up.";
        return true;
    }

    if (op == SandboxOp::Extract)
    {
        int value = 0;

        if (!m_heap.extractMax(value, log))
        {
            msg = "The active heap is empty.";
            return false;
        }

        msg = "Extracted " + num(value) + " into the sorted region.";
        return true;
    }

    if (op == SandboxOp::Build)
    {
        if (total == 0)
        {
            msg = "The array is empty.";
            return false;
        }

        m_heap.buildMaxHeap(log);
        msg = "Bottom-up Max Heap construction over the whole array.";
        return true;
    }

    if (op == SandboxOp::Heapify)
    {
        if (!m_heap.siftDown(a, active, log))
        {
            msg = "Index " + num(a) + " is outside the active heap (size " + num(active) + ").";
            return false;
        }

        msg = "Sift-down from index " + num(a) + ".";
        return true;
    }

    if (op == SandboxOp::SiftUp)
    {
        if (!m_heap.siftUp(a, log))
        {
            msg = "Index " + num(a) + " is outside the active heap (size " + num(active) + ").";
            return false;
        }

        msg = "Sift-up from index " + num(a) + ".";
        return true;
    }

    if (op == SandboxOp::Swap)
    {
        if (a == b)
        {
            msg = "Pick two different indices.";
            return false;
        }

        if (!m_heap.swapIdx(a, b, log))
        {
            msg = "Both indices must be inside the active heap (size " + num(active) + ").";
            return false;
        }

        msg = "Swapped cells " + num(a) + " and " + num(b) + ".";
        return true;
    }

    if (op == SandboxOp::Sort)
    {
        if (total == 0)
        {
            msg = "The array is empty.";
            return false;
        }
        
        m_heap.heapSort(log);
        msg = "Heap Sort complete: ascending (comparisons " + std::to_string((unsigned long long)m_heap.comparisonCount()) +
              ", swaps " + std::to_string((unsigned long long)m_heap.swapCount()) + ").";
        return true;
    }

    if (op == SandboxOp::Info)
    {
        msg = "Active heap " + num(active) + " of " + num(total) + " cells" +
              (active > 0 ? (", max = " + num(m_heap.array()[0])) : std::string("")) + ".";
        return true;
    }

    msg = "Not available for a heap.";
    return false;
}