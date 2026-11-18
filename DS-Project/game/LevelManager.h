#pragma once


#include "../contracts/GameTypes.h"
#include "../model/BinaryTree.h"
#include "../model/MaxHeap.h"

// Pure C++ campaign rules: alarm capacity, optimal steps, win checks, stars, unlocks.
// Stateless (all static). No SFML. Optimal steps are always computed from the model,
// never typed into the level files.
//
// Level index everywhere = (act - 1) * 5 + (level - 1), range 0..19.
class LevelManager
{
public:
    static const int LevelCount = 20;

    // ---- alarm / stars --------------------------------------------------
    // ceil(1.5 * optimal) + 2
    static int alarmCapacity(int optimalSteps);

    // 3 stars: steps <= optimal and no penalties
    // 2 stars: steps <= 1.25 * optimal and at most one penalty
    // 1 star : won within capacity.   0 stars: not won.
    // A hint caps the result at 2 stars.
    static int starsFor(bool won, int stepsTaken, int optimalSteps,
        int penalties, int penaltyCount, bool hintUsed);

    static bool isWon(const HudState& hud);
    static bool isLost(const HudState& hud);

    // ---- optimal steps --------------------------------------------------
    // Computes the optimal step count for a level. Acts 2-4 are asked from GameSession
    // (it owns the generated tree / heap); Act 1 is computed here from the generated
    // binary tree. Returns false if the level cannot be generated or the mission has
    // no fixed optimum (endless).
    //   traversal / reconstruct : one click or token per room
    //   heightDiameter          : one entry per room + the diameter report
    //   lcaPath                 : 2 selections + 1 LCA click + path rooms (rooms nodeCount-1 and nodeCount/2)
    //   mirror                  : one swap per room that has a child
    static bool optimalSteps(const LevelDef& level, int& out);

    // ---- win checks -----------------------------------------------------
    // Level 1.1: the answer must equal the requested traversal (param 0 pre, 1 in, 2 post, 3 level).
    static bool traversalMatches(const BinaryTree& tree, int traversalType, const IntArray& answer);
    // Level 1.2: the built tree equals the hidden tree.
    static bool reconstructMatches(const BinaryTree& hidden, const BinaryTree& built);
    // Level 1.5: current equals the mirror image of original.
    static bool mirrorMatches(const BinaryTree& original, const BinaryTree& current);
    // Level 4.x: build/repair need a valid heap; extract/race/boss need an ascending sorted array.
    static bool heapWon(int mission, const MaxHeap& heap);

    // ---- unlocks --------------------------------------------------------
    // Level 0 is always open. Others need >= 1 star on the previous level, unless unlockAll.
    static bool isUnlocked(const IntArray& stars, int index, bool unlockAll);
    static int nextLevel(int index);                 // kNull after the last level
    static int firstOpenLevel(const IntArray& stars);// first unlocked level with 0 stars, else the last unlocked
    static int totalStars(const IntArray& stars);
    static bool actCleared(const IntArray& stars, int act);   // act 1..4: every level has >= 1 star
};

