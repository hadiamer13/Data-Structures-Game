#include "TestSupport.h"
#include "../model/BST.h"
#include "../model/Validator.h"

static void seedBST(BST& tree)
{
    const int values[] = {8, 4, 12, 2, 6, 10, 14, 5};
    for (int i = 0; i < 8; i++)
    {
        tree.insert(values[i]);
    }
}

void runBSTTests()
{
    BST tree;
    seedBST(tree);
    testCheck(Validator::validateBST(tree), "insertions preserve BST order and parent links");
    testCheck(tree.root()->key == 8 && tree.search(5) != nullptr && tree.search(99) == nullptr, "BST search finds and misses keys");
    testCheck(!tree.insert(6) && tree.nodeCount() == 8, "BST rejects duplicate keys");
    testCheck(tree.findMin()->key == 2 && tree.findMax()->key == 14, "BST finds minimum and maximum");
    testCheck(tree.successor(6)->key == 8 && tree.predecessor(6)->key == 5, "BST computes successor and predecessor");
    testCheck(tree.successor(14) == nullptr && tree.predecessor(2) == nullptr, "BST reports missing boundary neighbors");
    IntArray range = tree.rangeQuery(5, 12);
    const int expected[] = {5, 6, 8, 10, 12};
    testCheck(range.size() == 5 && sameInts(range.data(), expected, 5), "range query returns sorted inclusive keys");
    testCheck(tree.kthSmallest(1) == 2 && tree.kthSmallest(8) == 14 && tree.kthSmallest(9) == kNull,
              "BST kthSmallest is one-based and bounds checked");

    EventLog log;
    testCheck(tree.remove(5, &log) && !tree.isAlive(7), "BST removes a leaf and retires its id");
    testCheck(Validator::validateBST(tree), "BST stays valid after leaf removal");
    BST oneChild;
    oneChild.insert(8);
    oneChild.insert(4);
    oneChild.insert(6);
    testCheck(oneChild.remove(4) && Validator::validateBST(oneChild), "BST removes a node with one right child");

    BST leftChild;
    leftChild.insert(8);
    leftChild.insert(4);
    testCheck(leftChild.remove(8) && leftChild.root()->key == 4 && leftChild.root()->parent == nullptr,
              "BST removes a root with only a left child");

    BST rightChild;
    rightChild.insert(8);
    rightChild.insert(12);
    testCheck(rightChild.remove(8) && rightChild.root()->key == 12 && rightChild.root()->parent == nullptr,
              "BST removes a root with only a right child");

    testCheck(tree.remove(8) && Validator::validateBST(tree), "BST removes a two-child root via its successor");
    const int beforeMissing = tree.nodeCount();
    testCheck(!tree.remove(404) && tree.nodeCount() == beforeMissing, "BST missing-key removal leaves the tree unchanged");
    testCheck(log.totalAlarmCost() > 0, "BST comparisons record alarm cost in the event log");
}
