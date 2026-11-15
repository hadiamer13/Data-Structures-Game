#include "../model/AVLTree.h"
#include "../model/Validator.h"
#include "TestSupport.h"

static void checkRotation(int first, int second, int third, int expectedRoot)
{
    AVLTree tree;
    testCheck(tree.insert(first), "AVL accepts first rotation-case key");
    testCheck(tree.insert(second), "AVL accepts second rotation-case key");
    testCheck(tree.insert(third), "AVL accepts third rotation-case key");
    testCheck(tree.root() != nullptr && tree.root()->key == expectedRoot,
              "AVL rotation places the expected key at the root");
    testCheck(Validator::validateAVL(tree), "AVL rotation preserves all invariants");
}

void runAVLTests()
{
    checkRotation(30, 20, 10, 20); // LL
    checkRotation(10, 20, 30, 20); // RR
    checkRotation(30, 10, 20, 20); // LR
    checkRotation(10, 30, 20, 20); // RL

    AVLTree deletionTree;
    const int deletionKeys[] = { 4, 2, 5, 1, 3 };
    for (int i = 0; i < 5; i++)
    {
        deletionTree.insert(deletionKeys[i]);
        testCheck(Validator::validateAVL(deletionTree), "AVL remains valid during setup");
    }
    testCheck(deletionTree.remove(5), "AVL removes the leaf that triggers a zero-balance heavy child");
    testCheck(deletionTree.root() != nullptr && deletionTree.root()->key == 2,
              "AVL deletion handles a heavy child with balance factor zero");
    testCheck(Validator::validateAVL(deletionTree), "AVL remains valid after zero-child-balance deletion");

    AVLTree removalTree;
    const int removalKeys[] = { 20, 10, 30, 25, 40, 22 };
    for (int i = 0; i < 6; i++)
    {
        removalTree.insert(removalKeys[i]);
    }
    const int removedId = removalTree.find(20)->id;
    testCheck(removalTree.remove(20), "AVL removes a two-child node");
    testCheck(!removalTree.isAlive(removedId) || removalTree.find(20) == nullptr,
              "AVL removes the requested key while preserving stable node ids");
    testCheck(Validator::validateAVL(removalTree), "AVL remains valid after two-child removal");
    testCheck(!removalTree.remove(999), "AVL reports a missing key without changing the tree");
    testCheck(Validator::validateAVL(removalTree), "AVL remains valid after a missing-key removal");

    AVLTree oneChildTree;
    oneChildTree.insert(8);
    oneChildTree.insert(4);
    testCheck(oneChildTree.remove(8), "AVL removes a root with one child");
    testCheck(oneChildTree.root() != nullptr && oneChildTree.root()->key == 4 &&
              oneChildTree.root()->parent == nullptr,
              "AVL promotes the only child to a parentless root");
    testCheck(Validator::validateAVL(oneChildTree), "AVL remains valid after one-child root removal");
    testCheck(oneChildTree.remove(4), "AVL removes its final leaf");
    testCheck(oneChildTree.root() == nullptr && Validator::validateAVL(oneChildTree),
              "AVL becomes a valid empty tree after the final removal");
}
