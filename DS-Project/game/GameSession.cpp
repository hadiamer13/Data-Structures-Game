#include "GameSession.h"
#include "LevelGenerator.h"
#include <cstdint>

namespace
{
    const int kWrongClickPenalty = 10;
    const int kWrongRotationPenalty = 50;
    const int kWrongRotationAlarm = 2;

    // Exactly one violated room (the root, id 0). Index = RotationType (LL, RR, LR, RL).
    const char* const kRotationTemplates[4] =
    {
        "50 30 20 10 # # # 40 # # 60 # #",
        "50 30 # # 70 60 # # 80 # 90 # #",
        "50 30 20 # # 40 35 # # # 60 # #",
        "50 30 # # 70 60 # 65 # # 80 # #"
    };

    void findDeepest(const TreeNode* node, int depth, int& bestDepth, int& bestKey)
    {
        if (node == nullptr)
        {
            return;
        }
        if (depth > bestDepth)
        {
            bestDepth = depth;
            bestKey = static_cast<const BSTNode*>(node)->key;
        }
        findDeepest(node->left, depth + 1, bestDepth, bestKey);
        findDeepest(node->right, depth + 1, bestDepth, bestKey);
    }

    int deleteCaseOf(const BSTNode* node)
    {
        if (node->left != nullptr && node->right != nullptr)
        {
            return DeleteCase::TwoChildren;
        }
        if (node->left == nullptr && node->right == nullptr)
        {
            return DeleteCase::Leaf;
        }
        return DeleteCase::OneChild;
    }

    std::uint32_t nextRandom(std::uint32_t& s)
    {
        s = s * 1103515245u + 12345u;
        return (s >> 16) & 0x7fffu;
    }

    int subSize(const TreeNode* n)
    {
        return n == nullptr ? 0 : static_cast<const AVLNode*>(n)->size;
    }

    void collectViolations(const TreeNode* n, IntArray& out)
    {
        if (n == nullptr)
        {
            return;
        }
        collectViolations(n->left, out);
        const int bf = AVLTree::balanceFactor(static_cast<const AVLNode*>(n));
        if (bf > 1 || bf < -1)
        {
            out.push(n->id);
        }
        collectViolations(n->right, out);
    }

    // Textbook case for a violated room, RotationType::None when |bf| <= 1.
    int correctCase(const AVLNode* n)
    {
        const int bf = AVLTree::balanceFactor(n);
        if (bf > 1)
        {
            return AVLTree::balanceFactor(static_cast<const AVLNode*>(n->left)) >= 0
                ? RotationType::LL : RotationType::LR;
        }
        if (bf < -1)
        {
            return AVLTree::balanceFactor(static_cast<const AVLNode*>(n->right)) <= 0
                ? RotationType::RR : RotationType::RL;
        }
        return RotationType::None;
    }

    const char* rotName(int r)
    {
        if (r == RotationType::LL)
        {
            return "LL";
        }
        if (r == RotationType::RR)
        {
            return "RR";
        }
        if (r == RotationType::LR)
        {
            return "LR";
        }
        if (r == RotationType::RL)
        {
            return "RL";
        }
        return "no rotation";
    }

    // Rotations that inserting the queue into a tree of startKeys would cause.
    int rotationsFor(const IntArray& startKeys, const IntArray& queue)
    {
        AVLTree scratch;
        for (int i = 0; i < startKeys.size(); i++)
        {
            scratch.insert(startKeys[i], nullptr);
        }
        EventLog log;
        for (int i = 0; i < queue.size(); i++)
        {
            scratch.insert(queue[i], &log);
        }
        int count = 0;
        for (int i = 0; i < log.size(); i++)
        {
            if (log.at(i).type == EventType::ROTATE)
            {
                count++;
            }
        }
        return count;
    }

    bool validRoom(const AVLTree& t, int id)
    {
        return id >= 0 && id < t.pool().size() && t.isAlive(id);
    }
}

GameSession::GameSession()
    : m_target(kNull), m_started(false), m_isAvl(false), m_queuePos(0),
    m_cursor(kNull), m_rankStart(0), m_rankLeft(0), m_k(kNull)
{
}

bool GameSession::isAvlMission(int mission)
{
    return mission == MissionKind::AvlBadges || mission == MissionKind::AvlSingle ||
        mission == MissionKind::AvlDouble || mission == MissionKind::AvlAuto ||
        mission == MissionKind::AvlOrderStat;
}

bool GameSession::supportsMission(int mission)
{
    return mission == MissionKind::BstSearch || mission == MissionKind::BstInsert ||
        mission == MissionKind::BstDelete || mission == MissionKind::BstBoss ||
        isAvlMission(mission);
}

int GameSession::pathCost(int key) const
{
    EventLog scratch;
    m_tree.search(key, &scratch);
    const int cost = scratch.totalAlarmCost();
    return cost < 1 ? 1 : cost;
}

// Level JSON "param" is used as the target key when it makes sense for the generated tree;
// otherwise the model picks one (deepest key to find / delete, max + 1 to insert).
int GameSession::pickTarget(const LevelDef& level) const
{
    const bool present = level.param != 0 && m_tree.search(level.param, nullptr) != nullptr;
    if (level.mission == MissionKind::BstInsert)
    {
        if (level.param != 0 && !present)
        {
            return level.param;
        }
        if (m_tree.root() == nullptr)
        {
            return 1;
        }
        return m_tree.findMax()->key + 1;
    }
    if (present)
    {
        return level.param;
    }
    if (m_tree.root() == nullptr)
    {
        return kNull;
    }
    int bestDepth = -1;
    int bestKey = kNull;
    findDeepest(m_tree.root(), 0, bestDepth, bestKey);
    return bestKey;
}

bool GameSession::startLevel(const LevelDef& level)
{
    m_tree.clear();
    m_avl.clear();
    m_undo.clear();
    m_lastLog.clear();
    m_level = level;
    m_target = kNull;
    m_isAvl = isAvlMission(level.mission);
    m_violIds.clear();
    m_found.clear();
    m_queue.clear();
    m_queuePos = 0;
    m_cursor = kNull;
    m_rankStart = 0;
    m_rankLeft = 0;
    m_k = kNull;

    if (m_isAvl)
    {
        return startAvl(level);
    }

    if (!level.preorder.empty())
    {
        if (!Serializer::deserializeBST(level.preorder, m_tree))
        {
            m_started = false;
            return false;
        }
    }
    else if (level.treeKind == TreeKind::BST && level.nodeCount > 0)
    {
        if (!LevelGenerator::generateBST(level, m_tree, nullptr))
        {
            m_tree.clear();
            m_started = false;
            return false;
        }
    }
    std::string error;
    if (!Validator::validateBST(m_tree, &error))
    {
        m_tree.clear();
        m_started = false;
        return false;
    }
    m_initialTree = Serializer::serializeBST(m_tree);

    int optimal = 1;
    if (supportsMission(level.mission))
    {
        m_target = pickTarget(level);
        if (m_target != kNull)
        {
            optimal = pathCost(m_target);   // optimal steps are computed by the model
        }
    }
    m_score.configure(optimal, level.objective);
    m_started = true;
    return true;
}

// ------------------------------------------------------------ AVL level setup

bool GameSession::buildBadgeTree(const LevelDef& level)
{
    LevelDef g = level;
    int n = g.nodeCount;
    if (n < 5)
    {
        n = 9;
    }
    if (n > 15)
    {
        n = 15;
    }
    g.nodeCount = n;

    // A random BST loaded as an AVL is usually unbalanced; a sorted one always is.
    for (int attempt = 0; attempt < 2; attempt++)
    {
        BST scratch;
        if (!LevelGenerator::generateBST(g, scratch, nullptr))
        {
            return false;
        }
        if (!Serializer::deserializeAVL(Serializer::serializeBST(scratch), m_avl))
        {
            return false;
        }
        IntArray bad;
        collectViolations(m_avl.root(), bad);
        if (bad.size() > 0)
        {
            return true;
        }
        g.style = 1;
    }
    return false;
}

bool GameSession::buildAutoTree(const LevelDef& level)
{
    int n = level.nodeCount;
    if (n < 3 || n > 12)
    {
        n = 7;
    }
    int len = level.param > 0 ? level.param : 6;
    if (len < 2)
    {
        len = 2;
    }
    if (len > 12)
    {
        len = 12;
    }

    IntArray startKeys;
    IntArray queue;
    for (int attempt = 0; attempt < 50; attempt++)
    {
        std::uint32_t state = (std::uint32_t)(level.seed + attempt) * 2654435761u + 1u;
        startKeys.clear();
        queue.clear();
        while (startKeys.size() < n)
        {
            const int key = (int)(nextRandom(state) % 90u) + 10;
            if (startKeys.indexOf(key) == -1)
            {
                startKeys.push(key);
            }
        }
        while (queue.size() < len)
        {
            const int key = (int)(nextRandom(state) % 90u) + 10;
            if (startKeys.indexOf(key) == -1 && queue.indexOf(key) == -1)
            {
                queue.push(key);
            }
        }
        if (rotationsFor(startKeys, queue) >= 2)
        {
            break;      // an interesting sequence: at least two rotations
        }
    }

    for (int i = 0; i < startKeys.size(); i++)
    {
        if (!m_avl.insert(startKeys[i], nullptr))
        {
            return false;
        }
    }
    m_queue = queue;
    return true;
}

bool GameSession::startAvl(const LevelDef& level)
{
    m_started = false;
    const int mission = level.mission;
    bool built = true;

    if (mission == MissionKind::AvlAuto)
    {
        built = buildAutoTree(level);          // a custom preorder is ignored here
    }
    else if (!level.preorder.empty())
    {
        built = Serializer::deserializeAVL(level.preorder, m_avl);
    }
    else if (mission == MissionKind::AvlBadges)
    {
        built = buildBadgeTree(level);
    }
    else if (mission == MissionKind::AvlSingle || mission == MissionKind::AvlDouble)
    {
        int c = level.param;
        if (mission == MissionKind::AvlSingle)
        {
            if (c != RotationType::LL && c != RotationType::RR)
            {
                c = (level.seed % 2 == 0) ? RotationType::LL : RotationType::RR;
            }
        }
        else if (c != RotationType::LR && c != RotationType::RL)
        {
            c = (level.seed % 2 == 0) ? RotationType::LR : RotationType::RL;
        }
        built = Serializer::deserializeAVL(kRotationTemplates[c], m_avl);
    }
    else
    {
        LevelDef g = level;                    // avlOrderStat: a balanced seeded tree
        if (g.nodeCount < 3 || g.nodeCount > 63)
        {
            g.nodeCount = 15;
        }
        built = LevelGenerator::generateAVL(g, m_avl, nullptr);
    }
    if (!built)
    {
        m_avl.clear();
        return false;
    }

    // Round trip so room ids are the preorder ids from the very start (stable after Reset).
    m_initialTree = Serializer::serializeAVL(m_avl);
    if (!Serializer::deserializeAVL(m_initialTree, m_avl))
    {
        m_avl.clear();
        return false;
    }

    int optimal = 1;
    if (mission == MissionKind::AvlBadges)
    {
        collectViolations(m_avl.root(), m_violIds);
        if (m_violIds.size() == 0)
        {
            return false;
        }
        optimal = m_violIds.size();
    }
    else if (mission == MissionKind::AvlSingle || mission == MissionKind::AvlDouble)
    {
        IntArray bad;
        collectViolations(m_avl.root(), bad);
        if (bad.size() == 0)
        {
            return false;      // nothing to repair
        }
        const int c = correctCase(static_cast<const AVLNode*>(m_avl.pool()[bad[0]]));
        optimal = (c == RotationType::LR || c == RotationType::RL) ? 2 : 1;
    }
    else
    {
        std::string error;
        if (!Validator::validateAVL(m_avl, &error))
        {
            m_avl.clear();
            return false;
        }
        if (mission == MissionKind::AvlAuto)
        {
            optimal = m_queue.size();
        }
        else
        {
            const int n = m_avl.nodeCount();
            m_k = (level.param >= 1 && level.param <= n) ? level.param : (n / 3 > 1 ? n / 3 : 1);
            m_rankStart = n - m_k + 1;     // k-th most valuable = (n - k + 1)-th smallest
            m_rankLeft = m_rankStart;
            // Optimal steps = rooms on the path from the root to the target.
            int rank = m_rankStart;
            const TreeNode* cur = m_avl.root();
            int steps = 1;
            while (cur != nullptr)
            {
                const int left = subSize(cur->left);
                if (rank == left + 1)
                {
                    break;
                }
                if (rank <= left)
                {
                    cur = cur->left;
                }
                else
                {
                    rank = rank - left - 1;
                    cur = cur->right;
                }
                steps++;
            }
            optimal = steps;
        }
    }

    m_score.configure(optimal, level.objective);
    m_started = true;
    return true;
}

// -------------------------------------------------------------- state helpers

void GameSession::resetMissionState()
{
    m_found.clear();
    m_cursor = kNull;
    m_rankLeft = m_rankStart;
    m_queuePos = 0;
}

void GameSession::restoreInitial()
{
    if (m_isAvl)
    {
        Serializer::deserializeAVL(m_initialTree, m_avl);
        resetMissionState();
        return;
    }
    Serializer::deserializeBST(m_initialTree, m_tree);
}

// Undo entry for AVL levels: "<queuePos>|<preorder>", so avlAuto undo also rewinds the queue.
std::string GameSession::captureAvl() const
{
    return std::to_string(m_queuePos) + "|" + Serializer::serializeAVL(m_avl);
}

bool GameSession::restoreAvl(const std::string& text)
{
    const size_t bar = text.find('|');
    if (bar == std::string::npos)
    {
        return false;
    }
    int pos = 0;
    for (size_t i = 0; i < bar; i++)
    {
        if (text[i] < '0' || text[i] > '9')
        {
            return false;
        }
        pos = pos * 10 + (text[i] - '0');
    }
    if (!Serializer::deserializeAVL(text.substr(bar + 1), m_avl))
    {
        return false;
    }
    m_queuePos = pos;
    return true;
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

// -------------------------------------------------------------------- apply

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
        const bool restored = m_isAvl ? restoreAvl(previous) : Serializer::deserializeBST(previous, m_tree);
        m_lastLog.clear();
        m_lastLog.beginBatch();
        return makeResult(restored, restored ? "Move undone" : "Undo snapshot was invalid");
    }
    if (action.kind == ActionKind::Hint)
    {
        m_score.useHint();
        m_lastLog.clear();
        m_lastLog.beginBatch();
        std::string text = "No hint for this level.";
        if (m_isAvl)
        {
            text = avlHint();
        }
        else if (m_target != kNull)
        {
            const std::string key = std::to_string(m_target);
            if (m_level.mission == MissionKind::BstInsert)
            {
                text = "Insert key " + key + ": smaller goes left, larger goes right, until a free slot.";
            }
            else
            {
                text = "Target key " + key + ": smaller goes left, larger goes right. (Star cap: 2)";
            }
        }
        return makeResult(true, text);
    }

    if (m_isAvl)
    {
        return applyAvl(action);
    }

    m_lastLog.clear();
    m_lastLog.beginBatch();
    bool accepted = false;
    bool changed = false;
    bool completed = false;
    int extraAlarm = 0;
    int penalty = 0;
    std::string message;
    const std::string before = Serializer::serializeBST(m_tree);
    const bool onTarget = m_target != kNull && action.a == m_target;

    if (action.kind == ActionKind::Insert)
    {
        accepted = m_tree.insert(action.a, &m_lastLog);
        changed = accepted;
        message = accepted ? "Key inserted" : "Duplicate key rejected";
        completed = accepted && (m_level.mission == MissionKind::BstInsert || m_level.mission == MissionKind::BstBoss) && onTarget;
    }
    else if (action.kind == ActionKind::Delete)
    {
        BSTNode* node = m_tree.search(action.a, nullptr);
        if (node != nullptr && action.c != kNull && action.c != deleteCaseOf(node))
        {
            // Level 2.3: the player must name the delete case first.
            accepted = false;
            extraAlarm = 1;
            penalty = 10;
            message = "Wrong delete case. +1 alarm";
        }
        else
        {
            accepted = m_tree.remove(action.a, &m_lastLog);
            changed = accepted;
            message = accepted ? "Key deleted" : "Key not found";
            completed = accepted && (m_level.mission == MissionKind::BstDelete || m_level.mission == MissionKind::BstBoss) && onTarget;
        }
    }
    else if (action.kind == ActionKind::Search)
    {
        BSTNode* found = m_tree.search(action.a, &m_lastLog);
        accepted = true;
        message = found == nullptr ? "Key not found" : "Key found";
        completed = found != nullptr && (m_level.mission == MissionKind::BstSearch || m_level.mission == MissionKind::BstBoss) && onTarget;
    }
    else
    {
        return makeResult(false, "Action is not supported in this phase");
    }

    if (changed)
    {
        m_undo.push(before);
    }
    const int alarmCost = m_lastLog.totalAlarmCost() + extraAlarm;
    m_score.recordAction(alarmCost, penalty);
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

// ---------------------------------------------------------------- AVL actions

ActionResult GameSession::applyAvl(const PlayerAction& action)
{
    m_lastLog.clear();
    m_lastLog.beginBatch();
    const int mission = m_level.mission;
    bool accepted = false;
    bool changed = false;
    bool completed = false;
    int stepAlarm = 0;
    int extraAlarm = 0;
    int penalty = 0;
    std::string message;
    const std::string before = captureAvl();

    if (mission == MissionKind::AvlBadges && action.kind == ActionKind::ClickNode)
    {
        const int id = action.a;
        if (!validRoom(m_avl, id))
        {
            return makeResult(false, "Click a room.");
        }
        if (m_found.indexOf(id) != -1)
        {
            return makeResult(false, "That room is already marked.");
        }
        const AVLNode* node = static_cast<const AVLNode*>(m_avl.pool()[id]);
        const int bf = AVLTree::balanceFactor(node);

        Event visit(EventType::VISIT);
        visit.id = id;
        visit.alarmCost = 1;
        emit(&m_lastLog, visit);

        if (bf > 1 || bf < -1)
        {
            m_found.push(id);
            Event alert(EventType::BALANCE_ALERT);
            alert.id = id;
            alert.newV = bf;
            emit(&m_lastLog, alert);
            accepted = true;
            completed = (m_found.size() == m_violIds.size());
            message = "Unbalanced room found (" + std::to_string(m_found.size()) + "/" + std::to_string(m_violIds.size()) + ")";
        }
        else
        {
            extraAlarm = 1;
            penalty = kWrongClickPenalty;
            message = "That room is balanced. +1 alarm";
        }
    }
    else if (mission == MissionKind::AvlOrderStat && action.kind == ActionKind::ClickNode)
    {
        const int id = action.a;
        if (!validRoom(m_avl, id))
        {
            return makeResult(false, "Click a room.");
        }
        const int expect = orderExpected();
        int leftSize = 0;
        bool goesRight = false;
        if (m_cursor != kNull)
        {
            const AVLNode* cur = static_cast<const AVLNode*>(m_avl.pool()[m_cursor]);
            leftSize = subSize(cur->left);
            goesRight = (m_rankLeft > leftSize + 1);
        }

        Event visit(EventType::VISIT);
        visit.id = id;
        visit.alarmCost = 1;
        emit(&m_lastLog, visit);

        if (id == expect)
        {
            if (goesRight)
            {
                m_rankLeft = m_rankLeft - leftSize - 1;
            }
            m_cursor = id;
            const AVLNode* node = static_cast<const AVLNode*>(m_avl.pool()[id]);
            accepted = true;
            completed = (m_rankLeft == subSize(node->left) + 1);
            message = completed ? "Found the treasure you were after!"
                : ("Correct. Left subtree has " + std::to_string(subSize(node->left)) + " rooms.");
        }
        else
        {
            extraAlarm = 1;
            penalty = kWrongClickPenalty;
            message = (m_cursor == kNull) ? "Start at the root. +1 alarm" : "Wrong room: compare the sizes again. +1 alarm";
        }
    }
    else if ((mission == MissionKind::AvlSingle || mission == MissionKind::AvlDouble) && action.kind == ActionKind::Rotate)
    {
        const int id = action.a;
        if (!validRoom(m_avl, id))
        {
            return makeResult(false, "Pick a room as the pivot.");
        }
        AVLNode* pivot = static_cast<AVLNode*>(m_avl.pool()[id]);
        const int correct = correctCase(pivot);

        if (correct == RotationType::None || action.b != correct)
        {
            extraAlarm = kWrongRotationAlarm;
            penalty = kWrongRotationPenalty;
            message = (correct == RotationType::None)
                ? "That room is not out of balance. +2 alarm, -50 points"
                : "Wrong rotation for this imbalance. +2 alarm, -50 points";
        }
        else
        {
            int rotations = 1;
            if (correct == RotationType::LL)
            {
                m_avl.rotateRight(pivot, &m_lastLog);
            }
            else if (correct == RotationType::RR)
            {
                m_avl.rotateLeft(pivot, &m_lastLog);
            }
            else if (correct == RotationType::LR)
            {
                AVLNode* child = static_cast<AVLNode*>(pivot->left);
                TreeNode* raised = child->right;
                m_avl.rotateLeft(child, &m_lastLog);      // may already finish the repair
                if (pivot->parent != raised)
                {
                    m_avl.rotateRight(pivot, &m_lastLog);
                }
                rotations = 2;
            }
            else
            {
                AVLNode* child = static_cast<AVLNode*>(pivot->right);
                TreeNode* raised = child->left;
                m_avl.rotateRight(child, &m_lastLog);
                if (pivot->parent != raised)
                {
                    m_avl.rotateLeft(pivot, &m_lastLog);
                }
                rotations = 2;
            }
            accepted = true;
            changed = true;
            stepAlarm = rotations;
            completed = m_avl.isBalanced();
            message = std::string(rotName(correct)) + " repair done" + (completed ? "." : ". Another room is still unbalanced.");
        }
    }
    else if (mission == MissionKind::AvlAuto && action.kind == ActionKind::Rotate)
    {
        if (m_queuePos >= m_queue.size())
        {
            return makeResult(false, "No more inserts.");
        }
        m_avl.insert(m_queue[m_queuePos], &m_lastLog);    // the vault always inserts
        int actual = RotationType::None;
        for (int i = 0; i < m_lastLog.size(); i++)
        {
            if (m_lastLog.at(i).type == EventType::ROTATE)
            {
                actual = m_lastLog.at(i).result;
                break;
            }
        }
        m_queuePos++;
        accepted = true;
        changed = true;
        completed = (m_queuePos >= m_queue.size());
        if (action.b == actual)
        {
            message = std::string("Correct: ") + rotName(actual) + ".";
        }
        else
        {
            extraAlarm = kWrongRotationAlarm;
            penalty = kWrongRotationPenalty;
            message = std::string("Wrong prediction: the vault did ") + rotName(actual) + ". +2 alarm, -50 points";
        }
    }
    else
    {
        return makeResult(false, "That action is not used in this mission.");
    }

    if (changed)
    {
        m_undo.push(before);
    }
    const int alarmCost = m_lastLog.totalAlarmCost() + stepAlarm + extraAlarm;
    m_score.recordAction(alarmCost, penalty);
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

// ------------------------------------------------------------------ queries

int GameSession::orderExpected() const
{
    if (m_avl.root() == nullptr)
    {
        return kNull;
    }
    if (m_cursor == kNull)
    {
        return m_avl.root()->id;
    }
    const AVLNode* cur = static_cast<const AVLNode*>(m_avl.pool()[m_cursor]);
    const int leftSize = subSize(cur->left);
    if (m_rankLeft == leftSize + 1)
    {
        return kNull;      // already at the target
    }
    if (m_rankLeft <= leftSize)
    {
        return cur->left == nullptr ? kNull : cur->left->id;
    }
    return cur->right == nullptr ? kNull : cur->right->id;
}

int GameSession::avlTargetNode() const
{
    const int mission = m_level.mission;
    if (mission == MissionKind::AvlBadges)
    {
        for (int i = 0; i < m_violIds.size(); i++)
        {
            if (m_found.indexOf(m_violIds[i]) == -1)
            {
                return m_violIds[i];
            }
        }
        return kNull;
    }
    if (mission == MissionKind::AvlSingle || mission == MissionKind::AvlDouble)
    {
        IntArray bad;
        collectViolations(m_avl.root(), bad);
        return bad.size() > 0 ? bad[0] : kNull;
    }
    if (mission == MissionKind::AvlOrderStat)
    {
        return orderExpected();
    }
    return kNull;
}

std::string GameSession::avlHint() const
{
    const int mission = m_level.mission;
    if (mission == MissionKind::AvlBadges)
    {
        return "Look for a room whose balance factor is beyond +1 or -1. (Star cap: 2)";
    }
    if (mission == MissionKind::AvlSingle)
    {
        return "The ringed room is out of balance. Which side is heavy? (Star cap: 2)";
    }
    if (mission == MissionKind::AvlDouble)
    {
        return "The ringed room is the pivot. Is its heavy child leaning the same way or the other way? (Star cap: 2)";
    }
    if (mission == MissionKind::AvlAuto)
    {
        return "Follow the new key down to its leaf, then find the first ancestor that leaves +/-1. (Star cap: 2)";
    }
    return "Compare your rank with the left subtree size at each room. The ringed room is the next step. (Star cap: 2)";
}

std::string GameSession::prompt() const
{
    if (!m_started)
    {
        return "";
    }
    const int mission = m_level.mission;
    if (mission == MissionKind::AvlBadges)
    {
        return "Unbalanced rooms found: " + std::to_string(m_found.size()) + " / " + std::to_string(m_violIds.size());
    }
    if (mission == MissionKind::AvlSingle || mission == MissionKind::AvlDouble)
    {
        return "Pick the pivot room, then choose the rotation.";
    }
    if (mission == MissionKind::AvlAuto)
    {
        if (m_queuePos >= m_queue.size())
        {
            return "All inserts done.";
        }
        return "Next insert: key " + std::to_string(m_queue[m_queuePos]) + " (" + std::to_string(m_queuePos + 1) + " of " +
            std::to_string(m_queue.size()) + "). Predict the rotation.";
    }
    if (mission == MissionKind::AvlOrderStat)
    {
        return "Steal the " + std::to_string(m_k) + "-th most valuable treasure. Click rooms from the root down.";
    }
    if (m_target != kNull)
    {
        return "Target key: " + std::to_string(m_target);
    }
    return "";
}

ViewSnapshot GameSession::snapshot() const
{
    return m_isAvl ? SnapshotBuilder::fromAVL(m_avl) : SnapshotBuilder::fromBST(m_tree);
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

const AVLTree& GameSession::avlTree() const
{
    return m_avl;
}

int GameSession::targetKey() const
{
    return m_isAvl ? kNull : m_target;
}

int GameSession::nextInsertKey() const
{
    if (!m_isAvl || m_level.mission != MissionKind::AvlAuto || m_queuePos >= m_queue.size())
    {
        return kNull;
    }
    return m_queue[m_queuePos];
}

int GameSession::orderK() const
{
    return (m_isAvl && m_level.mission == MissionKind::AvlOrderStat) ? m_k : kNull;
}

int GameSession::targetNodeId() const
{
    if (m_isAvl)
    {
        return avlTargetNode();
    }
    if (m_target == kNull || m_level.mission == MissionKind::BstInsert)
    {
        return kNull;
    }
    BSTNode* node = m_tree.search(m_target, nullptr);
    return node == nullptr ? kNull : node->id;
}