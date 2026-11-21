// contracts/GameTypes.h

#ifndef VH_GAMETYPES_H
#define VH_GAMETYPES_H

#include <string>
#include "Types.h"
#include "Event.h"
#include "Containers.h"

namespace ActionKind
{
    inline const int ClickNode = 0;
    inline const int SubmitSequence = 1;
    inline const int Insert = 2;
    inline const int Delete = 3;
    inline const int Search = 4;
    inline const int Range = 5;
    inline const int Rotate = 6;
    inline const int SwapHeap = 7;
    inline const int BuildHeap = 8;
    inline const int Heapify = 9;
    inline const int ExtractMax = 10;
    inline const int HeapSort = 11;
    inline const int Undo = 12;
    inline const int Reset = 13;
    inline const int Hint = 14;
    inline const int Place = 15;
    inline const int Count = 16;
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

struct HudState
{
    int alarm;
    int alarmCap;
    int score;
    int stepsTaken;
    int optimalSteps;
    int penalties;
    int stars;
    int status;
    std::string objective;

    HudState() : alarm(0), alarmCap(0), score(0), stepsTaken(0), optimalSteps(0), penalties(0), stars(0), status(HudStatus::Playing)
    {
    }
};

struct ActionResult
{
    bool accepted;
    EventArray events;
    std::string message;
    HudState hud;

    ActionResult() : accepted(false)
    {
    }
};

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

struct LevelDef
{
    int act;
    int index;
    std::string name;
    std::string conceptText;
    std::string objective;
    int treeKind;
    int mission;
    int seed;
    int nodeCount;
    int style;
    int param;
    bool boss;
    std::string preorder;

    LevelDef() : act(1), index(1), treeKind(TreeKind::Binary), mission(0), seed(1), nodeCount(7), style(0), param(0), boss(false)
    {
    }
};

#endif
