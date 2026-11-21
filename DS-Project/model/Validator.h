#ifndef VH_VALIDATOR_H
#define VH_VALIDATOR_H

#include <string>
#include "BinaryTree.h"
#include "BST.h"
#include "AVLTree.h"
#include "MaxHeap.h"

class Validator
{
  public:
    static bool validateBinaryTree(const BinaryTree& tree, std::string* error = nullptr);
    static bool validateBST(const BST& tree, std::string* error = nullptr);
    static bool validateAVL(const AVLTree& tree, std::string* error = nullptr);
    static bool validateMaxHeap(const MaxHeap& heap, std::string* error = nullptr);
};

#endif
