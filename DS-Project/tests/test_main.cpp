#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../third_party/doctest/doctest.h"

void runAVLTests();
void runOrderStatisticTests();
void runMaxHeapTests();
void runHeapSortTests();
void runBinaryTreeTests();
void runContainerTests();
void runNodePoolTests();
void runBSTTests();
void runSerializerTests();
void runValidatorTests();
void runScoreTests();
void runGameSessionTests();
void runSortTests();
void runCampaignSupportTests();
void runAct4SessionTests();

TEST_CASE("AVL tree operations")
{
    runAVLTests();
}

TEST_CASE("AVL order statistics")
{
    runOrderStatisticTests();
}

TEST_CASE("Max heap operations")
{
    runMaxHeapTests();
}

TEST_CASE("Heap sort")
{
    runHeapSortTests();
}

TEST_CASE("Binary tree operations")
{
    runBinaryTreeTests();
}

TEST_CASE("Fixed-size containers")
{
    runContainerTests();
}

TEST_CASE("Node pool ownership")
{
    runNodePoolTests();
}

TEST_CASE("BST operations")
{
    runBSTTests();
}

TEST_CASE("Tree and heap serializer")
{
    runSerializerTests();
}

TEST_CASE("Model validators")
{
    runValidatorTests();
}

TEST_CASE("Scoring rules")
{
    runScoreTests();
}

TEST_CASE("Game session bridge")
{
    runGameSessionTests();
}

TEST_CASE("Custom sorting algorithms")
{
    runSortTests();
}

TEST_CASE("Campaign JSON, generation, save, and leaderboard")
{
    runCampaignSupportTests();
}

TEST_CASE("Act 4 heap missions") 
{
    runAct4SessionTests(); 
}