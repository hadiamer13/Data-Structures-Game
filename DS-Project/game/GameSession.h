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

// Delete cases the player declares in PlayerAction::c (level 2.3).
namespace DeleteCase
{
    inline const int Leaf = 0;
    inline const int OneChild = 1;
    inline const int TwoChildren = 2;
}

// Playable bridge for the general binary tree missions (1.1 to 1.5), the BST missions
// (2.1, 2.2, 2.3, 2.5), the AVL missions (3.1 to 3.5) and the Max Heap missions (4.1 to 4.5).
// The GUI consumes ActionResult and snapshots; it never changes model or score state.
//
// Act 1 actions (BinaryTree):
//   traversal    (1.1): SubmitSequence seq = room ids in the order the player clicked them
//   heightDiameter (1.3): SubmitSequence a = reported height, b = reported diameter
//   lcaPath      (1.4): ClickNode a = room id clicked as the meeting room
//   mirror       (1.5): Rotate (mirrors the whole tree once; a second press mirrors it back)
//
// Act 2 actions (BST): see the comments in GameSession.cpp.
//
// Act 3 actions:
//   avlBadges    : ClickNode a = room id
//   avlSingle    : Rotate a = pivot id, b = RotationType::LL / RR
//   avlDouble    : Rotate a = pivot id, b = RotationType::LR / RL (both rotations run)
//   avlAuto      : Rotate b = predicted rotation (RotationType::None = "no rotation");
//                  the next queued key is inserted whatever the prediction
//   avlOrderStat : ClickNode a = room id, walking from the root down to the target
// Level JSON "param": avlSingle/avlDouble = RotationType, avlAuto = number of inserts,
// avlOrderStat = k (k-th most valuable). nodeCount = tree size.
//
// Act 4 actions (heap cell indices, 0-based):
//   heapBuild   (4.1) : SwapHeap a,b  |  Heapify a (sift-down from a)
//   heapRepair  (4.2) : SwapHeap a,b  |  Heapify a
//   heapExtract (4.3) : ExtractMax
//   heapRace    (4.4) : BuildHeap | ExtractMax | Heapify a | SwapHeap a,b   (op limit = param)
//   heapBoss    (4.5) : BuildHeap | ExtractMax | Heapify a | SwapHeap a,b
// A SwapHeap is valid only for a parent/child pair inside the active heap where the child is
// larger (a sift step). Invalid swaps are rejected: +1 alarm and a small penalty.
// HeapSort (the one-click action) is rejected in every mission.
// Level JSON: nodeCount = array size (3..31), param = operation limit for heapRace,
// style 5 = duplicate-heavy values for heapBoss.
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
    const MaxHeap& heap() const;    // Act 4: array() holds the active heap and the sorted suffix
    const BinaryTree& binaryTree() const;   // Act 1: the live general binary tree

    int targetKey() const;       // BST missions: target key, else kNull
    int targetNodeId() const;    // room / heap index the hint should ring, kNull if none
    int nextInsertKey() const;   // avlAuto: key that the next prediction inserts, else kNull
    int orderK() const;          // avlOrderStat: k, else kNull
    int lcaTargetA() const;      // lcaPath: first room the player is asked about, else kNull
    int lcaTargetB() const;      // lcaPath: second room the player is asked about, else kNull
    int opsUsed() const;         // heapRace: operations spent so far
    int opLimit() const;         // heapRace: operation limit, 0 for other missions
    std::string prompt() const;  // one-line instruction / progress for the GUI bar

    static bool supportsMission(int mission);
    static bool isBinaryMission(int mission);
    static bool isAvlMission(int mission);
    static bool isHeapMission(int mission);

    const BinaryTree& builtTree() const;     // 1.2: the tree the player has built so far
    bool isReconstruct() const;
    IntArray reconstructTokens() const;      // 1.2: values still on the shelf, ascending
    std::string reconstructClue() const;     // 1.2: "Preorder: ...   Inorder: ..."

    int rangeLo() const;         // bstRange: lower bound of the query, else kNull
    int rangeHi() const;         // bstRange: upper bound of the query, else kNull
    int rangeOptimal() const;    // bstRange: visits made by the unmodified query

private:
    std::string reconstructHint() const;
    bool nextPlacement(int& value, int& parentId, int& side) const;
    BinaryTree m_built;                      // 1.2: player's tree (m_bin stays the hidden answer)

    void restoreInitial();
    int pickTarget(const LevelDef& level) const;
    int pathCost(int key) const;
    ActionResult makeResult(bool accepted, const std::string& message) const;

    // ---- Act 1 general binary tree missions ----
    bool startBinary(const LevelDef& level);
    ActionResult applyBinary(const PlayerAction& action);

    // ---- AVL missions ----
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

    // ---- Heap missions ----
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

    IntArray m_violIds;   // avlBadges: unbalanced room ids at the start
    IntArray m_found;     // avlBadges: ids already marked
    IntArray m_queue;     // avlAuto: keys still to insert (full list)
    int m_queuePos;       // avlAuto: next index in m_queue
    int m_cursor;         // avlOrderStat: last correctly clicked room, kNull at the start
    int m_rankStart;      // avlOrderStat: target rank counted from the smallest
    int m_rankLeft;       // avlOrderStat: rank inside the cursor's subtree
    int m_k;              // avlOrderStat: k-th most valuable

    MaxHeap m_heap;           // Act 4 working heap (may be deliberately invalid mid-level)
    std::string m_initialHeap;// "<active> <total> v0 v1 ..." snapshot taken at level start
    bool m_isHeap;
    int m_opsUsed;
    int m_opLimit;

    BinaryTree m_bin;         // Act 1 working tree
    BinaryTree m_originalBin; // Act 1 unmirrored copy (serialize/deserialize), used by mirror check
    int m_lcaA;               // lcaPath: first room (n-1), same id LevelManager uses
    int m_lcaB;               // lcaPath: second room (n/2), same id LevelManager uses
    bool m_isBinary;

    IntArray m_rangeExpected;    // bstRange: keys that must be collected
    IntArray m_rangeFound;       // bstRange: keys the player has clicked correctly
    int m_rangeLo;
    int m_rangeHi;
    int m_rangeOptimal;

    ActionResult applyRange(const PlayerAction& action);
    std::string rangePrompt() const;
    bool isRangeMission() const;

};

#endif