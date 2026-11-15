#include "GameSession.h"

GameSession::GameSession() : m_started(false)
{
}

bool GameSession::startLevel(const LevelDef& level)
{
    m_tree.clear();
    m_undo.clear();
    m_lastLog.clear();
    m_level = level;
    if (!level.preorder.empty() && !Serializer::deserializeBST(level.preorder, m_tree))
    {
        m_started = false;
        return false;
    }
    std::string error;
    if (!Validator::validateBST(m_tree, &error))
    {
        m_tree.clear();
        m_started = false;
        return false;
    }
    m_initialTree = Serializer::serializeBST(m_tree);
    m_score.configure(1, level.objective);
    m_started = true;
    return true;
}

void GameSession::restoreInitial()
{
    Serializer::deserializeBST(m_initialTree, m_tree);
}

ActionResult GameSession::makeResult(bool accepted, const std::string& message) const
{
    ActionResult result;
    result.accepted = accepted;
    result.message = message;
    result.events = m_lastLog.events();
    result.hud = m_score.hud();
    return result;
}

ActionResult GameSession::apply(const PlayerAction& action)
{
    if (!m_started)
    {
        return makeResult(false, "No level is active");
    }
    if (m_score.status() != HudStatus::Playing && action.kind != ActionKind::Reset && action.kind != ActionKind::Undo)
    {
        return makeResult(false, "The level has ended");
    }

    if (action.kind == ActionKind::Reset)
    {
        restoreInitial();
        m_undo.clear();
        m_score.reset();
        m_lastLog.clear();
        m_lastLog.beginBatch();
        return makeResult(true, "Level reset");
    }
    if (action.kind == ActionKind::Undo)
    {
        if (m_undo.empty())
        {
            return makeResult(false, "Nothing to undo");
        }
        const std::string previous = m_undo.back();
        m_undo.pop();
        const bool restored = Serializer::deserializeBST(previous, m_tree);
        m_lastLog.clear();
        m_lastLog.beginBatch();
        return makeResult(restored, restored ? "Move undone" : "Undo snapshot was invalid");
    }

    m_lastLog.clear();
    m_lastLog.beginBatch();
    bool accepted = false;
    bool changed = false;
    bool completed = false;
    std::string message;
    const std::string before = Serializer::serializeBST(m_tree);
    if (action.kind == ActionKind::Insert)
    {
        accepted = m_tree.insert(action.a, &m_lastLog);
        changed = accepted;
        message = accepted ? "Key inserted" : "Duplicate key rejected";
        completed = accepted && (m_level.mission == MissionKind::BstInsert || m_level.mission == MissionKind::BstBoss) &&
                    (m_level.param == 0 || action.a == m_level.param);
    }
    else if (action.kind == ActionKind::Delete)
    {
        accepted = m_tree.remove(action.a, &m_lastLog);
        changed = accepted;
        message = accepted ? "Key deleted" : "Key not found";
        completed = accepted && (m_level.mission == MissionKind::BstDelete || m_level.mission == MissionKind::BstBoss) &&
                    (m_level.param == 0 || action.a == m_level.param);
    }
    else if (action.kind == ActionKind::Search)
    {
        BSTNode* found = m_tree.search(action.a, &m_lastLog);
        accepted = true;
        message = found == nullptr ? "Key not found" : "Key found";
        completed = found != nullptr && (m_level.mission == MissionKind::BstSearch || m_level.mission == MissionKind::BstBoss) &&
                    (m_level.param == 0 || action.a == m_level.param);
    }
    else
    {
        return makeResult(false, "Action is not supported in this phase");
    }

    if (changed)
    {
        m_undo.push(before);
    }
    const int alarmCost = m_lastLog.totalAlarmCost();
    m_score.recordAction(alarmCost);
    if (m_score.status() == HudStatus::Playing && completed)
    {
        m_score.markWon();
    }
    Event done(EventType::DONE);
    done.key = alarmCost;
    done.alarmCost = 0;
    emit(&m_lastLog, done);
    return makeResult(accepted, message);
}

ViewSnapshot GameSession::snapshot() const
{
    return SnapshotBuilder::fromBST(m_tree);
}

HudState GameSession::hud() const
{
    return m_score.hud();
}

const EventLog& GameSession::lastLog() const
{
    return m_lastLog;
}

bool GameSession::canUndo() const
{
    return !m_undo.empty();
}

const BST& GameSession::tree() const
{
    return m_tree;
}
