#include "LevelGenerator.h"

#include <cstdlib>

void LevelGenerator::seed(int value)
{
    std::srand(static_cast<unsigned int>(value));
}

void LevelGenerator::shuffle(IntArray& values)
{
    for (int i = values.size() - 1; i > 0; i--)
    {
        const int j = std::rand() % (i + 1);
        const int temporary = values[i];
        values[i] = values[j];
        values[j] = temporary;
    }
}

void LevelGenerator::makeKeys(int count, IntArray& keys, bool randomize)
{
    keys.clear();
    for (int i = 0; i < count; i++) keys.push(i + 1);
    if (randomize) shuffle(keys);
}

bool LevelGenerator::generate(const LevelDef& level, BinaryTree* binary, BST* bst,
                              AVLTree* avl, MaxHeap* heap, EventLog* log)
{
    if (level.treeKind == TreeKind::Binary) return binary != nullptr && generateBinaryTree(level, *binary);
    if (level.treeKind == TreeKind::BST) return bst != nullptr && generateBST(level, *bst, log);
    if (level.treeKind == TreeKind::AVL) return avl != nullptr && generateAVL(level, *avl, log);
    if (level.treeKind == TreeKind::MaxHeap) return heap != nullptr && generateHeap(level, *heap, log);
    return false;
}

bool LevelGenerator::generateBinaryTree(const LevelDef& level, BinaryTree& tree)
{
    if (level.nodeCount < 0 || level.nodeCount > 10000) return false;
    seed(level.seed);
    tree.clear();
    if (level.nodeCount == 0) return true;
    IntArray values;
    makeKeys(level.nodeCount, values, true);
    TreeNode* root = tree.createNode(values[0]);
    if (!tree.setRoot(root)) return false;
    NodePtrArray openParents;
    openParents.push(root);
    for (int i = 1; i < values.size(); i++)
    {
        const int parentIndex = std::rand() % openParents.size();
        TreeNode* parent = openParents[parentIndex];
        int side = Side::Left;
        if (parent->left != nullptr) side = Side::Right;
        else if (parent->right != nullptr && (std::rand() % 2) == 0) side = Side::Right;
        TreeNode* child = tree.createNode(values[i]);
        const bool attached = side == Side::Left ? tree.attachLeft(parent, child) : tree.attachRight(parent, child);
        if (!attached) return false;
        if (parent->left != nullptr && parent->right != nullptr)
        {
            for (int j = parentIndex; j + 1 < openParents.size(); j++) openParents[j] = openParents[j + 1];
            openParents.pop();
        }
        openParents.push(child);
    }
    return true;
}

bool LevelGenerator::generateBST(const LevelDef& level, BST& tree, EventLog* log)
{
    if (level.nodeCount < 0 || level.nodeCount > 10000) return false;
    seed(level.seed);
    tree.clear();
    IntArray keys;
    makeKeys(level.nodeCount, keys, level.style == 0);
    if (level.style == 3)
    {
        for (int i = 0; i < keys.size() / 2; i++)
        {
            const int temporary = keys[i];
            keys[i] = keys[keys.size() - 1 - i];
            keys[keys.size() - 1 - i] = temporary;
        }
    }
    for (int i = 0; i < keys.size(); i++)
    {
        if (!tree.insert(keys[i], log)) return false;
    }
    return true;
}

bool LevelGenerator::generateAVL(const LevelDef& level, AVLTree& tree, EventLog* log)
{
    if (level.nodeCount < 0 || level.nodeCount > 10000) return false;
    seed(level.seed);
    tree.clear();
    IntArray keys;
    makeKeys(level.nodeCount, keys, level.style == 0);
    if (level.style == 3)
    {
        for (int i = 0; i < keys.size() / 2; i++)
        {
            const int temporary = keys[i];
            keys[i] = keys[keys.size() - 1 - i];
            keys[keys.size() - 1 - i] = temporary;
        }
    }
    if ((level.mission == MissionKind::AvlSingle || level.mission == MissionKind::AvlDouble) &&
        level.nodeCount >= 3 && level.param >= RotationType::LL && level.param <= RotationType::RL)
    {
        int pattern[3];
        if (level.param == RotationType::LL) { pattern[0] = 30; pattern[1] = 20; pattern[2] = 10; }
        else if (level.param == RotationType::RR) { pattern[0] = 10; pattern[1] = 20; pattern[2] = 30; }
        else if (level.param == RotationType::LR) { pattern[0] = 30; pattern[1] = 10; pattern[2] = 20; }
        else { pattern[0] = 10; pattern[1] = 30; pattern[2] = 20; }
        for (int i = 0; i < 3; i++) if (!tree.insert(pattern[i], log)) return false;
        for (int i = 0; i < keys.size(); i++)
        {
            const int key = keys[i] + 100;
            if (!tree.insert(key, log)) return false;
        }
        return true;
    }
    for (int i = 0; i < keys.size(); i++) if (!tree.insert(keys[i], log)) return false;
    return true;
}

bool LevelGenerator::generateHeap(const LevelDef& level, MaxHeap& heap, EventLog* log)
{
    if (level.nodeCount < 0 || level.nodeCount > 100000) return false;
    seed(level.seed);
    IntArray values;
    for (int i = 0; i < level.nodeCount; i++)
    {
        int value = i + 1;
        if (level.style == 0) value = std::rand() % (level.nodeCount * 4 + 1);
        else if (level.style == 3) value = level.nodeCount - i;
        else if (level.style == 4) value = level.nodeCount - i;
        else if (level.style == 5) value = std::rand() % (level.nodeCount / 4 + 1);
        values.push(value);
    }
    if (level.style == 2)
    {
        for (int i = 0; i < values.size() / 2; i++)
        {
            const int temporary = values[i];
            values[i] = values[values.size() - 1 - i];
            values[values.size() - 1 - i] = temporary;
        }
    }
    heap.buildMaxHeap(values, log);
    return true;
}

bool LevelGenerator::generateAVLRotationShowcase(const LevelDef& level, AVLTree& tree, EventLog* log)
{
    seed(level.seed);
    for (int pass = 0; pass < 2; pass++)
    {
        for (int type = RotationType::LL; type <= RotationType::RL; type++)
        {
            tree.clear();
            const int offset = pass * 100;
            int pattern[3];
            if (type == RotationType::LL) { pattern[0] = 30; pattern[1] = 20; pattern[2] = 10; }
            else if (type == RotationType::RR) { pattern[0] = 10; pattern[1] = 20; pattern[2] = 30; }
            else if (type == RotationType::LR) { pattern[0] = 30; pattern[1] = 10; pattern[2] = 20; }
            else { pattern[0] = 10; pattern[1] = 30; pattern[2] = 20; }
            for (int i = 0; i < 3; i++)
            {
                if (!tree.insert(pattern[i] + offset, log)) return false;
            }
        }
    }
    return true;
}
