#ifndef VH_LEVELGENERATOR_H
#define VH_LEVELGENERATOR_H

#include "../contracts/GameTypes.h"
#include "../model/BinaryTree.h"
#include "../model/BST.h"
#include "../model/AVLTree.h"
#include "../model/MaxHeap.h"

// Seeded builders for the four campaign data-structure acts. This is the only
// game module that uses the C library random-number generator.
class LevelGenerator
{
public:
    static bool generate(const LevelDef& level, BinaryTree* binary, BST* bst,
                         AVLTree* avl, MaxHeap* heap, EventLog* log = nullptr);
    static bool generateBinaryTree(const LevelDef& level, BinaryTree& tree);
    static bool generateBST(const LevelDef& level, BST& tree, EventLog* log = nullptr);
    static bool generateAVL(const LevelDef& level, AVLTree& tree, EventLog* log = nullptr);
    static bool generateHeap(const LevelDef& level, MaxHeap& heap, EventLog* log = nullptr);
    static bool generateAVLRotationShowcase(const LevelDef& level, AVLTree& tree, EventLog* log = nullptr);

private:
    static void seed(int value);
    static void makeKeys(int count, IntArray& keys, bool shuffle);
    static void shuffle(IntArray& values);
};

#endif
