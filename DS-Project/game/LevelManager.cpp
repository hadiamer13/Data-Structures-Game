#include "LevelManager.h"

#include "GameSession.h"
#include "LevelGenerator.h"
#include "../model/Serializer.h"

namespace
{
    int commonPrefix(const IntArray& a, const IntArray& b)
    {
        int i = 0;
        while (i < a.size() && i < b.size() && a[i] == b[i])
        {
            i++;
        }
        return i;
    }

    bool sameSequence(const IntArray& a, const IntArray& b)
    {
        if (a.size() != b.size())
        {
            return false;
        }
        for (int i = 0; i < a.size(); i++)
        {
            if (a[i] != b[i])
            {
                return false;
            }
        }
        return true;
    }

    bool isBinaryMission(int mission)
    {
        return mission >= MissionKind::Traversal && mission <= MissionKind::Mirror;
    }
}

// ------------------------------------------------------------ alarm / stars

int LevelManager::alarmCapacity(int optimalSteps)
{
    const int optimal = optimalSteps < 1 ? 1 : optimalSteps;
    return (3 * optimal + 1) / 2 + 2;
}

int LevelManager::starsFor(bool won, int stepsTaken, int optimalSteps,
                           int penalties, int penaltyCount, bool hintUsed)
{
    if (!won)
    {
        return 0;
    }

    int result = 1;

    if (stepsTaken <= optimalSteps && penalties == 0)
    {
        result = 3;
    }

    else if (4 * stepsTaken <= 5 * optimalSteps && penaltyCount <= 1)
    {
        result = 2;
    }

    if (hintUsed && result > 2)
    {
        result = 2;
    }

    return result;
}

bool LevelManager::isWon(const HudState& hud)
{
    return hud.status == HudStatus::Won;
}

bool LevelManager::isLost(const HudState& hud)
{
    return hud.status == HudStatus::Lost;
}

// ------------------------------------------------------------ optimal steps

bool LevelManager::optimalSteps(const LevelDef& level, int& out)
{
    out = 1;
    const int mission = level.mission;

    if (isBinaryMission(mission))
    {
        BinaryTree tree;

        if (!level.preorder.empty())
        {
            if (!Serializer::deserializeBinaryTree(level.preorder, tree))
            {
                return false;
            }
        }

        else if (!LevelGenerator::generateBinaryTree(level, tree))
        {
            return false;
        }

        const int n = tree.nodeCount();

        if (n < 1)
        {
            return false;
        }

        if (mission == MissionKind::Traversal || mission == MissionKind::Reconstruct)
        {
            out = n;
        }

        else if (mission == MissionKind::HeightDiameter)
        {
            out = n + 1;
        }

        else if (mission == MissionKind::LcaPath)
        {
            const int a = n - 1;
            const int b = n / 2;
            IntArray pathA = tree.pathTo(a);
            IntArray pathB = tree.pathTo(b);
            const int shared = commonPrefix(pathA, pathB);
            const int pathRooms = (pathA.size() - shared) + (pathB.size() - shared) + 1;
            out = 3 + pathRooms;
        }

        else
        {
            int swaps = 0;
            const NodePtrArray& pool = tree.pool();
            for (int i = 0; i < pool.size(); i++)
            {
                if (tree.isAlive(i) && (pool[i]->left != nullptr || pool[i]->right != nullptr))
                {
                    swaps++;
                }
            }
            out = swaps < 1 ? 1 : swaps;
        }
        return true;
    }

    if (GameSession::supportsMission(mission))
    {
        GameSession session;

        if (!session.startLevel(level))
        {
            return false;
        }

        out = session.hud().optimalSteps;

        return true;
    }
    return false;
}

// --------------------------------------------------------------- win checks

bool LevelManager::traversalMatches(const BinaryTree& tree, int traversalType, const IntArray& answer)
{
    IntArray expected;

    if (traversalType == 0)
    {
        expected = tree.preorder();
    }
    else if (traversalType == 1)
    {
        expected = tree.inorder();
    }
    else if (traversalType == 2)
    {
        expected = tree.postorder();
    }
    else if (traversalType == 3)
    {
        expected = tree.levelOrder();
    }
    else
    {
        return false;
    }

    return sameSequence(expected, answer);
}

bool LevelManager::reconstructMatches(const BinaryTree& hidden, const BinaryTree& built)
{
    return hidden.isSameTree(built);
}

bool LevelManager::mirrorMatches(const BinaryTree& original, const BinaryTree& current)
{
    BinaryTree expected;

    if (!Serializer::deserializeBinaryTree(Serializer::serializeBinaryTree(original), expected))
    {
        return false;
    }

    expected.mirror();

    return expected.isSameTree(current);
}

bool LevelManager::heapWon(int mission, const MaxHeap& heap)
{
    if (mission == MissionKind::HeapBuild || mission == MissionKind::HeapRepair)
    {
        return heap.activeSize() == heap.totalSize() && heap.isMaxHeap();
    }

    if (mission == MissionKind::HeapExtract || mission == MissionKind::HeapRace ||
        mission == MissionKind::HeapBoss)
    {
        return heap.activeSize() == 0 && heap.isSortedAscending();
    }

    return false;
}

// ------------------------------------------------------------------ unlocks

bool LevelManager::isUnlocked(const IntArray& stars, int index, bool unlockAll)
{
    if (index < 0 || index >= LevelCount)
    {
        return false;
    }

    if (unlockAll || index == 0)
    {
        return true;
    }

    return stars.size() > index && stars[index - 1] >= 1;
}

int LevelManager::nextLevel(int index)
{
    if (index < 0 || index >= LevelCount - 1)
    {
        return kNull;
    }

    return index + 1;
}

int LevelManager::firstOpenLevel(const IntArray& stars)
{
    int lastUnlocked = 0;

    for (int i = 0; i < LevelCount; i++)
    {
        if (!isUnlocked(stars, i, false))
        {
            break;
        }
        lastUnlocked = i;
        if (stars.size() <= i || stars[i] == 0)
        {
            return i;
        }
    }

    return lastUnlocked;
}

int LevelManager::totalStars(const IntArray& stars)
{
    int total = 0;

    for (int i = 0; i < stars.size() && i < LevelCount; i++)
    {
        total += stars[i];
    }

    return total;
}

bool LevelManager::actCleared(const IntArray& stars, int act)
{
    if (act < 1 || act > 4 || stars.size() < LevelCount)
    {
        return false;
    }

    const int first = (act - 1) * 5;

    for (int i = first; i < first + 5; i++)
    {
        if (stars[i] < 1)
        {
            return false;
        }
    }
    
    return true;
}