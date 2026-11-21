#pragma once

#include "../contracts/GameTypes.h"
#include "../model/BinaryTree.h"
#include "../model/MaxHeap.h"

class LevelManager
{
  public:
    static const int LevelCount = 20;

    // ---- alarm / stars --------------------------------------------------
    static int alarmCapacity(int optimalSteps);

    static int starsFor(bool won, int stepsTaken, int optimalSteps,
                        int penalties, int penaltyCount, bool hintUsed);

    static bool isWon(const HudState& hud);
    static bool isLost(const HudState& hud);

    static bool optimalSteps(const LevelDef& level, int& out);

    // ---- win checks -----------------------------------------------------
    static bool traversalMatches(const BinaryTree& tree, int traversalType, const IntArray& answer);
    static bool reconstructMatches(const BinaryTree& hidden, const BinaryTree& built);
    static bool mirrorMatches(const BinaryTree& original, const BinaryTree& current);
    static bool heapWon(int mission, const MaxHeap& heap);

    // ---- unlocks --------------------------------------------------------

    static bool isUnlocked(const IntArray& stars, int index, bool unlockAll);
    static int nextLevel(int index);                  
    static int firstOpenLevel(const IntArray& stars); 
    static int totalStars(const IntArray& stars);
    static bool actCleared(const IntArray& stars, int act); 
};
