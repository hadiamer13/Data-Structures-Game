#include "TestSupport.h"
#include "../game/LevelManager.h"
#include "../game/LevelGenerator.h"
#include "../model/Serializer.h"

void runLevelManagerTests()
{
    // alarm capacity = ceil(1.5 x optimal) + 2
    testCheck(LevelManager::alarmCapacity(4) == 8, "capacity for optimal 4 is 8");
    testCheck(LevelManager::alarmCapacity(1) == 4, "capacity for optimal 1 is 4");
    testCheck(LevelManager::alarmCapacity(3) == 7, "capacity rounds 4.5 up to 5, plus two");
    testCheck(LevelManager::alarmCapacity(0) == 4, "optimal below one is treated as one");

    // stars
    testCheck(LevelManager::starsFor(false, 1, 4, 0, 0, false) == 0, "a lost level earns no stars");
    testCheck(LevelManager::starsFor(true, 4, 4, 0, 0, false) == 3, "optimal and clean earns three stars");
    testCheck(LevelManager::starsFor(true, 5, 4, 10, 1, false) == 2, "1.25 x optimal with one penalty earns two stars");
    testCheck(LevelManager::starsFor(true, 5, 4, 20, 2, false) == 1, "two penalties earn one star");
    testCheck(LevelManager::starsFor(true, 4, 4, 0, 0, true) == 2, "a hint caps the result at two stars");

    // unlocks
    IntArray stars;
    for (int i = 0; i < 20; i++)
    {
        stars.push(0);
    }
    testCheck(LevelManager::isUnlocked(stars, 0, false), "level 1.1 is always open");
    testCheck(!LevelManager::isUnlocked(stars, 1, false), "level 1.2 is locked at the start");
    testCheck(LevelManager::isUnlocked(stars, 7, true), "unlock-all opens every level");
    testCheck(!LevelManager::isUnlocked(stars, 20, true), "an out-of-range index is never open");
    stars[0] = 1;
    testCheck(LevelManager::isUnlocked(stars, 1, false), "one star unlocks the next level");
    testCheck(LevelManager::firstOpenLevel(stars) == 1, "first open level is the first unplayed one");
    stars[1] = 3;
    testCheck(LevelManager::totalStars(stars) == 4, "total stars add up");
    testCheck(LevelManager::nextLevel(3) == 4 && LevelManager::nextLevel(19) == kNull, "next level stops after 4.5");
    testCheck(!LevelManager::actCleared(stars, 1), "act 1 is not cleared with two levels done");
    for (int i = 0; i < 5; i++)
    {
        stars[i] = 1;
    }
    testCheck(LevelManager::actCleared(stars, 1) && !LevelManager::actCleared(stars, 2), "act clear needs every level");

    // optimal steps (model computed)
    LevelDef traversal;
    traversal.treeKind = TreeKind::Binary;
    traversal.mission = MissionKind::Traversal;
    traversal.seed = 101;
    traversal.nodeCount = 8;
    int optimal = 0;
    testCheck(LevelManager::optimalSteps(traversal, optimal) && optimal == 8, "traversal optimum is one click per room");

    LevelDef diameter = traversal;
    diameter.mission = MissionKind::HeightDiameter;
    testCheck(LevelManager::optimalSteps(diameter, optimal) && optimal == 9, "height levels add the diameter report");

    LevelDef lca = traversal;
    lca.mission = MissionKind::LcaPath;
    testCheck(LevelManager::optimalSteps(lca, optimal) && optimal >= 4, "LCA levels cost selections, the LCA click and the path");

    LevelDef mirror = traversal;
    mirror.mission = MissionKind::Mirror;
    testCheck(LevelManager::optimalSteps(mirror, optimal) && optimal >= 1 && optimal < 8, "mirror optimum counts rooms with children");

    LevelDef search;
    search.treeKind = TreeKind::BST;
    search.mission = MissionKind::BstSearch;
    search.param = 12;
    search.preorder = "8 4 # # 12 # #";
    testCheck(LevelManager::optimalSteps(search, optimal) && optimal == 2, "BST search optimum is the comparison count");

    LevelDef endless;
    endless.mission = MissionKind::Endless;
    testCheck(!LevelManager::optimalSteps(endless, optimal), "endless has no fixed optimum");

    // win checks
    BinaryTree tree;
    LevelGenerator::generateBinaryTree(traversal, tree);
    IntArray pre = tree.preorder();
    testCheck(LevelManager::traversalMatches(tree, 0, pre), "the exact preorder wins level 1.1");
    testCheck(!LevelManager::traversalMatches(tree, 1, pre) || tree.inorder().size() != pre.size() ||
        tree.preorder().size() == 1, "a preorder answer does not satisfy an inorder question on this tree");
    pre[0] = pre[1];
    testCheck(!LevelManager::traversalMatches(tree, 0, pre), "a wrong sequence fails");

    BinaryTree copy;
    Serializer::deserializeBinaryTree(Serializer::serializeBinaryTree(tree), copy);
    testCheck(LevelManager::reconstructMatches(tree, copy), "an identical tree wins level 1.2");
    testCheck(!LevelManager::mirrorMatches(tree, copy) || tree.isSymmetric(), "an unmirrored copy fails level 1.5");
    copy.mirror();
    testCheck(LevelManager::mirrorMatches(tree, copy), "a mirrored copy wins level 1.5");

    // heap win checks
    IntArray values;
    values.push(3);
    values.push(9);
    values.push(5);
    MaxHeap heap;
    heap.buildMaxHeap(values);
    testCheck(LevelManager::heapWon(MissionKind::HeapBuild, heap), "a built heap wins 4.1");
    testCheck(!LevelManager::heapWon(MissionKind::HeapBoss, heap), "an unsorted heap does not win 4.5");
    heap.heapSort();
    testCheck(LevelManager::heapWon(MissionKind::HeapBoss, heap), "a sorted array wins 4.5");
}