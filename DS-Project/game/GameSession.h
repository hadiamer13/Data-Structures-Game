#ifndef VH_GAMESESSION_H
#define VH_GAMESESSION_H

#include "../contracts/GameTypes.h"
#include "../model/BinaryTree.h"
#include "../model/BST.h"
#include "../model/AVLTree.h"
#include "../model/MaxHeap.h"
#include "../model/Serializer.h"
#include "../model/Validator.h"
#include "SnapshotBuilder.h"
#include "ScoreSystem.h"

namespace DeleteCase
{
    inline const int Leaf = 0;
    inline const int OneChild = 1;
    inline const int TwoChildren = 2;
}

class GameSession
{
  public:
    GameSession();

    bool startLevel(const LevelDef& level);
    ActionResult apply(const PlayerAction& action);
    ViewSnapshot snapshot() const;
    HudState hud() const;
    const EventLog& lastLog() const;
    bool canUndo() const;
    const BST& tree() const;
    const AVLTree& avlTree() const;
    const MaxHeap& heap() const;
    const BinaryTree& binaryTree() const;

    int targetKey() const;
    int targetNodeId() const;
    int nextInsertKey() const;
    int orderK() const;
    int lcaTargetA() const;
    int lcaTargetB() const;
    int opsUsed() const;
    int opLimit() const;
    std::string prompt() const;

    static bool supportsMission(int mission);
    static bool isBinaryMission(int mission);
    static bool isAvlMission(int mission);
    static bool isHeapMission(int mission);

    const BinaryTree& builtTree() const;
    bool isReconstruct() const;
    IntArray reconstructTokens() const;
    std::string reconstructClue() const;

    int rangeLo() const;
    int rangeHi() const;
    int rangeOptimal() const;

  private:
    std::string reconstructHint() const;
    bool nextPlacement(int& value, int& parentId, int& side) const;
    BinaryTree m_built;

    void restoreInitial();
    int pickTarget(const LevelDef& level) const;
    int pathCost(int key) const;
    ActionResult makeResult(bool accepted, const std::string& message) const;

    bool startBinary(const LevelDef& level);
    ActionResult applyBinary(const PlayerAction& action);

    bool startAvl(const LevelDef& level);
    bool buildBadgeTree(const LevelDef& level);
    bool buildAutoTree(const LevelDef& level);
    ActionResult applyAvl(const PlayerAction& action);
    std::string captureAvl() const;
    bool restoreAvl(const std::string& text);
    void resetMissionState();
    int orderExpected() const;
    int avlTargetNode() const;
    std::string avlHint() const;

    bool startHeap(const LevelDef& level);
    ActionResult applyHeap(const PlayerAction& action);
    std::string captureHeap() const;
    bool restoreHeap(const std::string& text);
    std::string heapHint() const;
    std::string heapPrompt() const;
    int heapTargetIndex() const;

    LevelDef m_level;
    BST m_tree;
    AVLTree m_avl;
    ScoreSystem m_score;
    EventLog m_lastLog;
    StringArray m_undo;
    std::string m_initialTree;
    int m_target;
    bool m_started;
    bool m_isAvl;

    IntArray m_violIds;
    IntArray m_found;
    IntArray m_queue;
    int m_queuePos;
    int m_cursor;
    int m_rankStart;
    int m_rankLeft;
    int m_k;

    MaxHeap m_heap;
    std::string m_initialHeap;
    bool m_isHeap;
    int m_opsUsed;
    int m_opLimit;

    BinaryTree m_bin;
    BinaryTree m_originalBin;
    int m_lcaA;
    int m_lcaB;
    bool m_isBinary;

    IntArray m_rangeExpected;
    IntArray m_rangeFound;
    int m_rangeLo;
    int m_rangeHi;
    int m_rangeOptimal;

    ActionResult applyRange(const PlayerAction& action);
    std::string rangePrompt() const;
    bool isRangeMission() const;
};

#endif