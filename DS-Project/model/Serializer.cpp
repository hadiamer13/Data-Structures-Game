#include "Serializer.h"

#include <sstream>

void Serializer::writeBinary(const TreeNode* node, std::ostringstream& out)
{
    if (node == nullptr)
    {
        out << "# ";
        return;
    }
    
    out << node->value << ' ';
    writeBinary(node->left, out);
    writeBinary(node->right, out);
}

void Serializer::writeBST(const BSTNode* node, std::ostringstream& out)
{
    if (node == nullptr)
    {
        out << "# ";
        return;
    }
    out << node->key << ' ';
    writeBST(node->left == nullptr ? nullptr : static_cast<const BSTNode*>(node->left), out);
    writeBST(node->right == nullptr ? nullptr : static_cast<const BSTNode*>(node->right), out);
}

void Serializer::writeAVL(const AVLNode* node, std::ostringstream& out)
{
    if (node == nullptr)
    {
        out << "# ";
        return;
    }
    out << node->key << ' ';
    writeAVL(node->left == nullptr ? nullptr : static_cast<const AVLNode*>(node->left), out);
    writeAVL(node->right == nullptr ? nullptr : static_cast<const AVLNode*>(node->right), out);
}

std::string Serializer::serializeBinaryTree(const BinaryTree& tree)
{
    std::ostringstream out;
    writeBinary(tree.root(), out);
    return out.str();
}

TreeNode* Serializer::readBinary(std::istream& in, BinaryTree& tree, TreeNode* parent,
                                 int side, bool& ok)
{
    std::string token;
    if (!(in >> token))
    {
        ok = false;
        return nullptr;
    }
    if (token == "#")
    {
        return nullptr;
    }
    std::istringstream valueStream(token);
    int value = 0;
    char extra = 0;
    if (!(valueStream >> value) || (valueStream >> extra))
    {
        ok = false;
        return nullptr;
    }
    TreeNode* node = tree.createNode(value);
    if (parent == nullptr)
    {
        ok = tree.setRoot(node);
    }
    else if (side == Side::Left)
    {
        ok = tree.attachLeft(parent, node);
    }
    else
    {
        ok = tree.attachRight(parent, node);
    }
    if (!ok)
    {
        return nullptr;
    }
    node->left = readBinary(in, tree, node, Side::Left, ok);
    if (!ok)
    {
        return nullptr;
    }
    node->right = readBinary(in, tree, node, Side::Right, ok);
    return node;
}

bool Serializer::deserializeBinaryTree(const std::string& text, BinaryTree& tree)
{
    tree.clear();
    std::istringstream in(text);
    bool ok = true;
    readBinary(in, tree, nullptr, Side::None, ok);
    std::string extra;
    if (in >> extra)
    {
        ok = false;
    }
    if (!ok)
    {
        tree.clear();
    }
    return ok;
}

std::string Serializer::serializeBST(const BST& tree)
{
    std::ostringstream out;
    writeBST(tree.root(), out);
    return out.str();
}

BSTNode* Serializer::readBST(std::istream& in, BST& tree, BSTNode* parent, int side,
                             bool hasLower, int lower, bool hasUpper, int upper, bool& ok)
{
    std::string token;
    if (!(in >> token))
    {
        ok = false;
        return nullptr;
    }
    if (token == "#")
    {
        return nullptr;
    }
    std::istringstream valueStream(token);
    int key = 0;
    char extra = 0;
    if (!(valueStream >> key) || (valueStream >> extra) ||
        (hasLower && key <= lower) || (hasUpper && key >= upper))
    {
        ok = false;
        return nullptr;
    }
    BSTNode* node = tree.m_pool.createBST(key);
    if (parent == nullptr)
    {
        tree.m_root = node;
    }
    else
    {
        node->parent = parent;
        if (side == Side::Left)
        {
            parent->left = node;
        }
        else
        {
            parent->right = node;
        }
    }
    node->left = readBST(in, tree, node, Side::Left, hasLower, lower, true, key, ok);
    if (!ok)
    {
        return nullptr;
    }
    node->right = readBST(in, tree, node, Side::Right, true, key, hasUpper, upper, ok);
    return node;
}

bool Serializer::deserializeBST(const std::string& text, BST& tree)
{
    tree.clear();
    std::istringstream in(text);
    bool ok = true;
    readBST(in, tree, nullptr, Side::None, false, 0, false, 0, ok);
    std::string extra;
    if (in >> extra)
    {
        ok = false;
    }
    if (!ok)
    {
        tree.clear();
    }
    return ok;
}

std::string Serializer::serializeAVL(const AVLTree& tree)
{
    std::ostringstream out;
    writeAVL(tree.root(), out);
    return out.str();
}

AVLNode* Serializer::readAVL(std::istream& in, AVLTree& tree, AVLNode* parent, int side,
                             bool hasLower, int lower, bool hasUpper, int upper, bool& ok)
{
    std::string token;
    if (!(in >> token))
    {
        ok = false;
        return nullptr;
    }
    if (token == "#")
    {
        return nullptr;
    }
    std::istringstream valueStream(token);
    int key = 0;
    char extra = 0;
    if (!(valueStream >> key) || (valueStream >> extra) ||
        (hasLower && key <= lower) || (hasUpper && key >= upper))
    {
        ok = false;
        return nullptr;
    }
    AVLNode* node = tree.m_pool.createAVL(key);
    if (parent == nullptr)
    {
        tree.m_root = node;
    }
    else
    {
        node->parent = parent;
        if (side == Side::Left)
        {
            parent->left = node;
        }
        else
        {
            parent->right = node;
        }
    }
    node->left = readAVL(in, tree, node, Side::Left, hasLower, lower, true, key, ok);
    if (!ok)
    {
        return nullptr;
    }
    node->right = readAVL(in, tree, node, Side::Right, true, key, hasUpper, upper, ok);
    if (!ok)
    {
        return nullptr;
    }
    const int leftHeight = node->left == nullptr ? 0 : static_cast<AVLNode*>(node->left)->height;
    const int rightHeight = node->right == nullptr ? 0 : static_cast<AVLNode*>(node->right)->height;
    const int leftSize = node->left == nullptr ? 0 : static_cast<AVLNode*>(node->left)->size;
    const int rightSize = node->right == nullptr ? 0 : static_cast<AVLNode*>(node->right)->size;
    node->height = (leftHeight > rightHeight ? leftHeight : rightHeight) + 1;
    node->size = leftSize + rightSize + 1;
    return node;
}

bool Serializer::deserializeAVL(const std::string& text, AVLTree& tree)
{
    tree.clear();
    std::istringstream in(text);
    bool ok = true;
    readAVL(in, tree, nullptr, Side::None, false, 0, false, 0, ok);
    std::string extra;
    if (in >> extra)
    {
        ok = false;
    }
    if (!ok)
    {
        tree.clear();
    }
    return ok;
}

std::string Serializer::serializeHeap(const MaxHeap& heap)
{
    std::ostringstream out;
    out << heap.activeSize() << ' ' << heap.totalSize() << ' ';
    for (int i = 0; i < heap.totalSize(); i++)
    {
        out << heap.array()[i] << ' ';
    }
    return out.str();
}

bool Serializer::deserializeHeap(const std::string& text, MaxHeap& heap)
{
    std::istringstream in(text);
    int activeSize = -1;
    int totalSize = -1;
    if (!(in >> activeSize >> totalSize) || activeSize < 0 || totalSize < 0 || activeSize > totalSize)
    {
        return false;
    }
    IntArray values;
    for (int i = 0; i < totalSize; i++)
    {
        int value = 0;
        if (!(in >> value))
        {
            return false;
        }
        values.push(value);
    }
    std::string extra;
    if (in >> extra)
    {
        return false;
    }
    return heap.loadState(values, activeSize);
}
