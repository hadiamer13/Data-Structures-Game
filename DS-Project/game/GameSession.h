#ifndef VH_GAMESESSION_H
#define VH_GAMESESSION_H

#include "../contracts/GameTypes.h"
#include "../model/BST.h"
#include "../model/AVLTree.h"
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

// Playable bridge for the BST missions (2.1, 2.2, 2.3, 2.5) and the AVL missions (3.1 to 3.5).
// The GUI consumes ActionResult and snapshots; it never changes model or score state.
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

    int targetKey() const;       // BST missions: target key, else kNull
    int targetNodeId() const;    // room the hint should ring, kNull if none
    int nextInsertKey() const;   // avlAuto: key that the next prediction inserts, else kNull
    int orderK() const;          // avlOrderStat: k, else kNull
    std::string prompt() const;  // one-line instruction / progress for the GUI bar

    static bool supportsMission(int mission);
    static bool isAvlMission(int mission);

private:
    void restoreInitial();
    int pickTarget(const LevelDef& level) const;
    int pathCost(int key) const;
    ActionResult makeResult(bool accepted, const std::string& message) const;

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
};

#endif