#ifndef VH_GAMESESSION_H
#define VH_GAMESESSION_H

#include "../contracts/GameTypes.h"
#include "../model/BST.h"
#include "../model/Serializer.h"
#include "../model/Validator.h"
#include "SnapshotBuilder.h"
#include "ScoreSystem.h"

// First playable bridge: BST insert, delete, search, undo and reset. The GUI
// consumes ActionResult and snapshots; it never changes model or score state.
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

private:
    void restoreInitial();
    ActionResult makeResult(bool accepted, const std::string& message) const;

    LevelDef m_level;
    BST m_tree;
    ScoreSystem m_score;
    EventLog m_lastLog;
    StringArray m_undo;
    std::string m_initialTree;
    bool m_started;
};

#endif
