// contracts/GameTypes.h
// Declarations shared by game/ (GameSession) and gui/ so both can be built in parallel.
// Pipeline: Input -> PlayerAction -> GameSession::apply() -> ActionResult{events,hud}.
#ifndef VH_GAMETYPES_H
#define VH_GAMETYPES_H

#include <string>
#include "Types.h"
#include "Event.h"
#include "Containers.h"

namespace ActionKind
{
    inline const int ClickNode = 0;       // a = node id
    inline const int SubmitSequence = 1;  // seq = ids (traversal / reconstruction answer)
    inline const int Insert = 2;          // a = key
    inline const int Delete = 3;          // a = key
    inline const int Search = 4;          // a = key
    inline const int Range = 5;           // a = lo, b = hi
    inline const int Rotate = 6;          // a = pivot id, b = RotationType
    inline const int SwapHeap = 7;        // a = index i, b = index j
    inline const int BuildHeap = 8;
    inline const int Heapify = 9;         // a = start index
    inline const int ExtractMax = 10;
    inline const int HeapSort = 11;
    inline const int Undo = 12;
    inline const int Reset = 13;
    inline const int Hint = 14;
    inline const int Count = 15;
}

namespace HudStatus
{
    inline const int Playing = 0;
    inline const int Won = 1;
    inline const int Lost = 2;
}

struct PlayerAction
{
    int kind;
    int a;
    int b;
    int c;
    IntArray seq;

    PlayerAction() : kind(ActionKind::ClickNode), a(kNull), b(kNull), c(kNull)
    {
    }
};

// Everything the HUD shows. Computed by ScoreSystem/LevelManager, never by GUI maths.
struct HudState
{
    int alarm;
    int alarmCap;
    int score;
    int stepsTaken;
    int optimalSteps;
    int penalties;
    int stars;
    int status;           // HudStatus::*
    std::string objective;

    HudState() : alarm(0), alarmCap(0), score(0), stepsTaken(0), optimalSteps(0), penalties(0), stars(0), status(HudStatus::Playing)
    {
    }
};

struct ActionResult
{
    bool accepted;
    EventArray events;
    std::string message;  // short text for the player ("Duplicate key rejected", ...)
    HudState hud;

    ActionResult() : accepted(false)
    {
    }
};

// Mission kinds (one per level type). Level JSON refers to these by name; see LevelSchema.md.
namespace MissionKind
{
    inline const int Traversal = 0;
    inline const int Reconstruct = 1;
    inline const int HeightDiameter = 2;
    inline const int LcaPath = 3;
    inline const int Mirror = 4;
    inline const int BstSearch = 5;
    inline const int BstInsert = 6;
    inline const int BstDelete = 7;
    inline const int BstRange = 8;
    inline const int BstBoss = 9;
    inline const int AvlBadges = 10;
    inline const int AvlSingle = 11;
    inline const int AvlDouble = 12;
    inline const int AvlAuto = 13;
    inline const int AvlOrderStat = 14;
    inline const int HeapBuild = 15;
    inline const int HeapRepair = 16;
    inline const int HeapExtract = 17;
    inline const int HeapRace = 18;
    inline const int HeapBoss = 19;
    inline const int Endless = 20;
}

// One level, loaded from JSON (game/LevelLoader fills it).
struct LevelDef
{
    int act;            // 1..4
    int index;          // 1..5 within the act
    std::string name;
    std::string conceptText;
    std::string objective;
    int treeKind;       // TreeKind::*
    int mission;        // MissionKind::*
    int seed;           // srand(seed) at generation
    int nodeCount;      // size of generated tree / array
    int style;          // generator hint: 0 random, 1 skewed, 2 sorted, 3 reverse, 4 already-heap, 5 duplicate-heavy
    int param;          // mission-specific (traversal type, k, op limit, ...)
    bool boss;
    std::string preorder; // optional explicit tree as "50 30 # # 70 # #"; empty = generate from seed

    LevelDef() : act(1), index(1), treeKind(TreeKind::Binary), mission(0), seed(1), nodeCount(7), style(0), param(0), boss(false)
    {
    }
};

#endif
