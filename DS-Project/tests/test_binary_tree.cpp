#include "../model/BinaryTree.h"
#include "../model/Validator.h"
#include "TestSupport.h"

static bool sameIds(const IntArray& left, const IntArray& right)
{
    if (left.size() != right.size())
    {
        return false;
    }
    for (int i = 0; i < left.size(); i++)
    {
        if (left[i] != right[i])
        {
            return false;
        }
    }
    return true;
}

static bool traversalValuesMatch(const BinaryTree& tree, const IntArray& ids,
                                 const int* expected, int count)
{
    if (ids.size() != count)
    {
        return false;
    }
    for (int i = 0; i < count; i++)
    {
        TreeNode* node = tree.pool()[ids[i]];
        if (node == nullptr || node->value != expected[i])
        {
            return false;
        }
    }
    return true;
}

static int findId(const BinaryTree& tree, int value)
{
    for (int i = 0; i < tree.pool().size(); i++)
    {
        TreeNode* node = tree.pool()[i];
        if (node != nullptr && node->value == value)
        {
            return node->id;
        }
    }
    return kNull;
}

void runBinaryTreeTests()
{
    BinaryTree empty;
    testCheck(empty.height() == 0 && empty.diameter() == 0, "empty tree has zero height and diameter");
    testCheck(empty.preorderRecursive().empty(), "empty tree traversal returns no nodes");
    testCheck(Validator::validateBinaryTree(empty), "empty tree passes validator");
    IntArray emptyValues;
    testCheck(empty.build(emptyValues, emptyValues), "empty traversal pair reconstructs an empty tree");

    const int preorderValues[] = {1, 2, 4, 5, 3, 6, 7};
    const int inorderValues[] = {4, 2, 5, 1, 6, 3, 7};
    const int postorderValues[] = {4, 5, 2, 6, 7, 3, 1};
    IntArray preorderInput;
    IntArray inorderInput;
    IntArray postorderInput;
    for (int i = 0; i < 7; i++)
    {
        preorderInput.push(preorderValues[i]);
        inorderInput.push(inorderValues[i]);
        postorderInput.push(postorderValues[i]);
    }

    BinaryTree preInTree;
    testCheck(preInTree.buildFromPreIn(preorderInput, inorderInput), "tree reconstructs from preorder and inorder");
    testCheck(preInTree.height() == 3, "full tree height counts nodes");
    testCheck(preInTree.diameter() == 4, "full tree diameter counts edges");
    testCheck(traversalValuesMatch(preInTree, preInTree.preorderRecursive(), preorderValues, 7),
              "recursive preorder matches input");
    testCheck(sameIds(preInTree.preorderRecursive(), preInTree.preorderIterative()),
              "recursive and iterative preorder agree");
    testCheck(sameIds(preInTree.inorderRecursive(), preInTree.inorderIterative()),
              "recursive and iterative inorder agree");
    testCheck(sameIds(preInTree.postorderRecursive(), preInTree.postorderIterative()),
              "recursive and iterative postorder agree");
    testCheck(sameIds(preInTree.levelOrderRecursive(), preInTree.levelOrderIterative()),
              "recursive and iterative level order agree");
    testCheck(traversalValuesMatch(preInTree, preInTree.inorder(), inorderValues, 7),
              "inorder matches input");
    testCheck(traversalValuesMatch(preInTree, preInTree.postorder(), postorderValues, 7),
              "postorder matches expected sequence");
    testCheck(Validator::validateBinaryTree(preInTree), "reconstructed full tree passes validator");

    BinaryTree postInTree;
    testCheck(postInTree.buildFromPostIn(postorderInput, inorderInput), "tree reconstructs from postorder and inorder");
    testCheck(preInTree.isSameTree(postInTree), "preorder/inorder and postorder/inorder round trip structurally");

    const int skewPre[] = {1, 2, 3};
    const int skewIn[] = {1, 2, 3};
    IntArray skewPreInput;
    IntArray skewInInput;
    for (int i = 0; i < 3; i++)
    {
        skewPreInput.push(skewPre[i]);
        skewInInput.push(skewIn[i]);
    }
    BinaryTree skewed;
    testCheck(skewed.build(skewPreInput, skewInInput), "right-skewed tree reconstructs");
    testCheck(skewed.height() == 3 && skewed.diameter() == 2, "skewed tree height and diameter are correct");
    testCheck(sameIds(skewed.levelOrderRecursive(), skewed.levelOrderIterative()),
              "skewed recursive and iterative level order agree");
    testCheck(Validator::validateBinaryTree(skewed), "skewed tree passes validator");

    const int id4 = findId(preInTree, 4);
    const int id5 = findId(preInTree, 5);
    const int id1 = findId(preInTree, 1);
    testCheck(preInTree.lca(id4, id5) == findId(preInTree, 2), "LCA returns the shared parent");
    testCheck(preInTree.lca(id4, findId(preInTree, 7)) == id1, "LCA returns the root across subtrees");
    const IntArray path = preInTree.pathTo(findId(preInTree, 7));
    testCheck(path.size() == 3 && path[0] == id1 && path[2] == findId(preInTree, 7),
              "pathTo returns root-to-node ids");

    BinaryTree symmetric;
    TreeNode* root = symmetric.createNode(1);
    TreeNode* left = symmetric.createNode(2);
    TreeNode* right = symmetric.createNode(2);
    TreeNode* leftOuter = symmetric.createNode(3);
    TreeNode* rightOuter = symmetric.createNode(3);
    testCheck(symmetric.setRoot(root), "manual tree accepts its root");
    testCheck(symmetric.attachLeft(root, left), "manual tree accepts a left child");
    testCheck(symmetric.attachRight(root, right), "manual tree accepts a right child");
    testCheck(symmetric.attachLeft(left, leftOuter), "manual tree accepts left outer node");
    testCheck(symmetric.attachRight(right, rightOuter), "manual tree accepts right outer node");
    testCheck(symmetric.isSymmetric(), "tree identifies mirror symmetry");
    symmetric.mirror();
    testCheck(symmetric.isSymmetric(), "mirror preserves symmetric structure");
    testCheck(Validator::validateBinaryTree(symmetric), "manual mirrored tree passes validator");

    IntArray duplicateValues;
    duplicateValues.push(1);
    duplicateValues.push(1);
    IntArray duplicateInorder;
    duplicateInorder.push(1);
    duplicateInorder.push(1);
    testCheck(!empty.build(duplicateValues, duplicateInorder), "ambiguous duplicate traversal values are rejected");
}
