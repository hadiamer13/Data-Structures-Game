#include "TestSupport.h"
#include "../model/Serializer.h"
#include "../model/Validator.h"

void runSerializerTests()
{
    BinaryTree source;
    TreeNode* root = source.createNode(50);
    TreeNode* left = source.createNode(30);
    TreeNode* right = source.createNode(70);
    TreeNode* inner = source.createNode(60);
    source.setRoot(root);
    source.attachLeft(root, left);
    source.attachRight(root, right);
    source.attachLeft(right, inner);
    const std::string binaryText = Serializer::serializeBinaryTree(source);
    BinaryTree binaryCopy;
    testCheck(Serializer::deserializeBinaryTree(binaryText, binaryCopy), "general tree serializer parses its preorder format");
    testCheck(Validator::validateBinaryTree(binaryCopy) && source.isSameTree(binaryCopy), "general tree round trip preserves shape and values");
    testCheck(!Serializer::deserializeBinaryTree("1 #", binaryCopy) && binaryCopy.root() == nullptr,
              "general tree serializer rejects truncated input safely");

    BST bst;
    bst.insert(50);
    bst.insert(30);
    bst.insert(70);
    bst.insert(60);
    const std::string bstText = Serializer::serializeBST(bst);
    BST bstCopy;
    testCheck(Serializer::deserializeBST(bstText, bstCopy) && Validator::validateBST(bstCopy), "BST serializer round-trips valid trees");
    testCheck(Serializer::serializeBST(bstCopy) == bstText, "BST round trip preserves exact shape");
    testCheck(!Serializer::deserializeBST("10 12 # # #", bstCopy) && bstCopy.root() == nullptr,
              "BST deserializer rejects invalid ordering");

    const int heapValues[] = { 9, 6, 8, 1, 4, 3, 7 };
    IntArray values;
    for (int i = 0; i < 7; i++) values.push(heapValues[i]);
    MaxHeap heap;
    heap.buildMaxHeap(values);
    int discarded = 0;
    heap.extractMax(discarded);
    const std::string heapText = Serializer::serializeHeap(heap);
    MaxHeap heapCopy;
    testCheck(Serializer::deserializeHeap(heapText, heapCopy), "heap serializer restores active and sorted regions");
    testCheck(heapCopy.activeSize() == heap.activeSize() && heapCopy.totalSize() == heap.totalSize() &&
              sameInts(heapCopy.array().data(), heap.array().data(), heap.totalSize()), "heap round trip preserves the full backing array");
    testCheck(!Serializer::deserializeHeap("3 2 1 2", heapCopy), "heap deserializer rejects inconsistent sizes");
    testCheck(!Serializer::deserializeHeap("2 2 1 2", heapCopy) && heapCopy.array()[0] == heap.array()[0],
              "heap deserializer rejects invalid order without altering the loaded heap");
}
