#ifndef VH_SERIALIZER_H
#define VH_SERIALIZER_H

#include <string>
#include <iosfwd>
#include "BinaryTree.h"
#include "BST.h"
#include "AVLTree.h"
#include "MaxHeap.h"

class Serializer
{
  public:
    static std::string serializeBinaryTree(const BinaryTree& tree);
    static bool deserializeBinaryTree(const std::string& text, BinaryTree& tree);
    static std::string serializeBST(const BST& tree);
    static bool deserializeBST(const std::string& text, BST& tree);
    static std::string serializeAVL(const AVLTree& tree);
    static bool deserializeAVL(const std::string& text, AVLTree& tree);
    static std::string serializeHeap(const MaxHeap& heap);
    static bool deserializeHeap(const std::string& text, MaxHeap& heap);

  private:
    static void writeBinary(const TreeNode* node, std::ostringstream& out);
    static void writeBST(const BSTNode* node, std::ostringstream& out);
    static void writeAVL(const AVLNode* node, std::ostringstream& out);
    static TreeNode* readBinary(std::istream& in, BinaryTree& tree, TreeNode* parent, int side, bool& ok);
    static BSTNode* readBST(std::istream& in, BST& tree, BSTNode* parent, int side,
                            bool hasLower, int lower, bool hasUpper, int upper, bool& ok);
    static AVLNode* readAVL(std::istream& in, AVLTree& tree, AVLNode* parent, int side,
                            bool hasLower, int lower, bool hasUpper, int upper, bool& ok);
};

#endif
