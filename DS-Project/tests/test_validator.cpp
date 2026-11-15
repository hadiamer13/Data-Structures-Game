#include "TestSupport.h"
#include "../model/Validator.h"

void runValidatorTests()
{
    BST tree;
    tree.insert(10);
    tree.insert(5);
    tree.insert(15);
    std::string error;
    testCheck(Validator::validateBST(tree, &error) && error.empty(), "BST validator accepts a valid tree");
    tree.root()->left->parent = nullptr;
    testCheck(!Validator::validateBST(tree, &error) && !error.empty(), "BST validator detects broken parent links");
    tree.root()->left->parent = tree.root();
    static_cast<BSTNode*>(tree.root()->left)->key = 20;
    testCheck(!Validator::validateBST(tree, &error), "BST validator detects ordering violations");
    static_cast<BSTNode*>(tree.root()->left)->key = 5;
    tree.root()->left->left = tree.root();
    testCheck(!Validator::validateBST(tree, &error), "BST validator detects cycles and shared nodes");
}
