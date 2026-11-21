#include "GameSession.h"
#include "LevelGenerator.h"
#include "LevelManager.h"
#include <cstdint>
#include <sstream>

namespace
{
    const int kWrongClickPenalty = 10;
    const int kWrongRotationPenalty = 50;
    const int kWrongRotationAlarm = 2;

    const char* const kRotationTemplates[4] =
        {
            "50 30 20 10 # # # 40 # # 60 # #",
            "50 30 # # 70 60 # # 80 # 90 # #",
            "50 30 20 # # 40 35 # # # 60 # #",
            "50 30 # # 70 60 # 65 # # 80 # #"};

    void decodeRange(int param, int& lo, int& hi)
    {
        lo = param / 1000;
        hi = param % 1000;
    }

    int findValueId(const BinaryTree& t, int value)
    {
        const NodePtrArray& pool = t.pool();
        for (int i = 0; i < pool.size(); i++)
        {
            if (t.isAlive(i) && pool[i]->value == value)
            {
                return i;
            }
        }
        return kNull;
    }

    const TreeNode* hiddenCounterpart(const BinaryTree& hidden, const TreeNode* node)
    {
        int sides[256];
        int depth = 0;
        const TreeNode* cur = node;

        while (cur->parent != nullptr)
        {
            if (depth >= 256)
            {
                return nullptr;
            }

            sides[depth++] = (cur->parent->left == cur) ? Side::Left : Side::Right;
            cur = cur->parent;
        }

        const TreeNode* h = hidden.root();

        for (int i = depth - 1; i >= 0 && h != nullptr; i--)
        {
            h = (sides[i] == Side::Left) ? h->left : h->right;
        }

        return h;
    }

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

    int correctCase(const AVLNode* n)
    {
        const int bf = AVLTree::balanceFactor(n);

        if (bf > 1)
        {
            return AVLTree::balanceFactor(static_cast<const AVLNode*>(n->left)) >= 0
                       ? RotationType::LL
                       : RotationType::LR;
        }

        if (bf < -1)
        {
            return AVLTree::balanceFactor(static_cast<const AVLNode*>(n->right)) <= 0
                       ? RotationType::RR
                       : RotationType::RL;
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

    bool buildBinaryTree(const LevelDef& level, BinaryTree& tree)
    {
        tree.clear();

        if (!level.preorder.empty())
        {
            return Serializer::deserializeBinaryTree(level.preorder, tree);
        }

        return LevelGenerator::generateBinaryTree(level, tree);
    }
}

GameSession::GameSession()
    : m_target(kNull), m_started(false), m_isAvl(false), m_queuePos(0),
      m_cursor(kNull), m_rankStart(0), m_rankLeft(0), m_k(kNull), m_isHeap(false), m_opsUsed(0), m_opLimit(0),
      m_lcaA(kNull), m_lcaB(kNull), m_isBinary(false)
{
}

bool GameSession::isBinaryMission(int mission)
{
    return mission >= MissionKind::Traversal && mission <= MissionKind::Mirror;
}

bool GameSession::isAvlMission(int mission)
{
    return mission == MissionKind::AvlBadges || mission == MissionKind::AvlSingle ||
           mission == MissionKind::AvlDouble || mission == MissionKind::AvlAuto ||
           mission == MissionKind::AvlOrderStat;
}

bool GameSession::supportsMission(int mission)
{
    return isBinaryMission(mission) || mission == MissionKind::BstSearch ||
           mission == MissionKind::BstInsert || mission == MissionKind::BstDelete ||
           mission == MissionKind::BstRange || mission == MissionKind::BstBoss ||
           isAvlMission(mission) || isHeapMission(mission);
}

int GameSession::pathCost(int key) const
{
    EventLog scratch;
    m_tree.search(key, &scratch);
    const int cost = scratch.totalAlarmCost();
    return cost < 1 ? 1 : cost;
}

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
    m_rangeExpected.clear();
    m_rangeFound.clear();
    m_rangeLo = kNull;
    m_rangeHi = kNull;
    m_rangeOptimal = 0;
    m_tree.clear();
    m_avl.clear();
    m_bin.clear();
    m_originalBin.clear();
    m_built.clear();
    m_undo.clear();
    m_lastLog.clear();
    m_level = level;
    m_target = kNull;
    m_isBinary = isBinaryMission(level.mission);
    m_isAvl = isAvlMission(level.mission);
    m_isHeap = isHeapMission(level.mission);
    m_heap.loadRaw(IntArray(), 0);
    m_initialHeap.clear();
    m_opsUsed = 0;
    m_opLimit = 0;
    m_violIds.clear();
    m_found.clear();
    m_queue.clear();
    m_queuePos = 0;
    m_cursor = kNull;
    m_rankStart = 0;
    m_rankLeft = 0;
    m_k = kNull;
    m_lcaA = kNull;
    m_lcaB = kNull;

    if (m_isBinary)
    {
        return startBinary(level);
    }

    if (m_isAvl)
    {
        return startAvl(level);
    }

    if (m_isHeap)
    {
        return startHeap(level);
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

    if (level.mission == MissionKind::BstRange)
    {
        decodeRange(level.param, m_rangeLo, m_rangeHi);

        if (m_rangeLo > m_rangeHi)
        {
            m_tree.clear();
            m_started = false;
            return false;
        }

        m_rangeExpected = m_tree.rangeQuery(m_rangeLo, m_rangeHi, nullptr);

        EventLog scratch;
        m_tree.rangeQuery(m_rangeLo, m_rangeHi, &scratch);
        m_rangeOptimal = scratch.totalAlarmCost();

        if (m_rangeOptimal < 1)
        {
            m_rangeOptimal = 1;
        }

        m_score.configure(m_rangeOptimal, level.objective);
        m_started = true;
        return true;
    }

    int optimal = 1;

    if (supportsMission(level.mission))
    {
        m_target = pickTarget(level);

        if (m_target != kNull)
        {
            optimal = pathCost(m_target);
        }
    }

    m_score.configure(optimal, level.objective);
    m_started = true;
    return true;
}

bool GameSession::startBinary(const LevelDef& level)
{
    m_started = false;

    if (!buildBinaryTree(level, m_bin))
    {
        m_bin.clear();
        return false;
    }

    if (!Serializer::deserializeBinaryTree(Serializer::serializeBinaryTree(m_bin), m_originalBin))
    {
        m_bin.clear();
        return false;
    }

    const int n = m_bin.nodeCount();

    if (n < 1)
    {
        m_bin.clear();
        return false;
    }

    m_lcaA = n - 1;
    m_lcaB = n / 2;

    int optimal = 1;

    if (!LevelManager::optimalSteps(level, optimal))
    {
        optimal = 1;
    }

    m_isBinary = true;
    m_isAvl = false;
    m_isHeap = false;
    m_score.configure(optimal, level.objective);
    m_started = true;
    return true;
}

ActionResult GameSession::applyBinary(const PlayerAction& action)
{
    m_lastLog.clear();
    m_lastLog.beginBatch();
    const int mission = m_level.mission;
    bool accepted = false;
    bool changed = false;
    bool completed = false;
    int extraAlarm = 0;
    int penalty = 0;
    std::string message;
    std::string before;

    if (mission == MissionKind::Traversal && action.kind == ActionKind::SubmitSequence)
    {
        accepted = LevelManager::traversalMatches(m_bin, m_level.param, action.seq);
        completed = accepted;
        message = accepted ? "Patrol route matches." : "That order does not match the patrol.";
        if (!accepted)
        {
            extraAlarm = 1;
            penalty = kWrongClickPenalty;
        }
    }

    else if (mission == MissionKind::HeightDiameter && action.kind == ActionKind::SubmitSequence)
    {
        const bool heightOk = action.a == m_bin.height();
        const bool diameterOk = action.b == m_bin.diameter();
        accepted = heightOk && diameterOk;
        completed = accepted;
        message = accepted ? "Height and diameter confirmed." : "One of those values is wrong.";

        if (!accepted)
        {
            extraAlarm = 1;
            penalty = kWrongClickPenalty;
        }
    }

    else if (mission == MissionKind::LcaPath && action.kind == ActionKind::ClickNode)
    {
        if (!validRoom(m_avl, action.a) && (action.a < 0 || action.a >= m_bin.pool().size() || !m_bin.isAlive(action.a)))
        {
            return makeResult(false, "Click a room.");
        }

        Event visit(EventType::VISIT);
        visit.id = action.a;
        visit.alarmCost = 1;
        emit(&m_lastLog, visit);

        const int common = m_bin.lca(m_lcaA, m_lcaB, nullptr);
        accepted = (action.a == common);
        completed = accepted;
        message = accepted ? "Meeting room found." : "That is not where the routes meet.";

        if (!accepted)
        {
            extraAlarm = 1;
            penalty = kWrongClickPenalty;
        }
    }
    else if (mission == MissionKind::Mirror && action.kind == ActionKind::Rotate)
    {
        m_bin.mirror(&m_lastLog);
        accepted = true;
        changed = true;
        completed = LevelManager::mirrorMatches(m_originalBin, m_bin);
        message = "Children swapped.";
    }
    else if (mission == MissionKind::Reconstruct && action.kind == ActionKind::Place)
    {
        const int value = action.a;
        const int parentId = action.b;
        const int side = action.c;

        if (findValueId(m_bin, value) == kNull || findValueId(m_built, value) != kNull)
        {
            return makeResult(false, "That token is not on the shelf.");
        }

        TreeNode* parent = nullptr;

        if (parentId == kNull)
        {
            if (m_built.root() != nullptr)
            {
                return makeResult(false, "The root is already placed.");
            }
        }

        else
        {
            if (parentId < 0 || parentId >= m_built.pool().size() || !m_built.isAlive(parentId))
            {
                return makeResult(false, "Drop the token on a placed room.");
            }

            parent = m_built.pool()[parentId];

            if ((side != Side::Left && side != Side::Right) ||
                (side == Side::Left ? parent->left : parent->right) != nullptr)
            {
                return makeResult(false, "That slot is not free.");
            }
        }

        bool correct = false;

        if (parent == nullptr)
        {
            correct = m_bin.root() != nullptr && m_bin.root()->value == value;
        }

        else
        {
            const TreeNode* h = hiddenCounterpart(m_bin, parent);
            if (h != nullptr)
            {
                const TreeNode* c = (side == Side::Left) ? h->left : h->right;
                correct = c != nullptr && c->value == value;
            }
        }

        if (correct)
        {
            before = Serializer::serializeBinaryTree(m_built);
            TreeNode* node = m_built.createNode(value);

            if (parent == nullptr)
            {
                m_built.setRoot(node, &m_lastLog);
            }

            else if (side == Side::Left)
            {
                m_built.attachLeft(parent, node, &m_lastLog);
            }

            else
            {
                m_built.attachRight(parent, node, &m_lastLog);
            }

            accepted = true;
            changed = true;
            extraAlarm = 1;
            completed = m_built.nodeCount() == m_bin.nodeCount() && LevelManager::reconstructMatches(m_bin, m_built);
            message = "Token " + std::to_string(value) + " placed.";
        }

        else
        {
            extraAlarm = 1;
            penalty = kWrongClickPenalty;
            message = "That token does not belong in that slot. +1 alarm";
        }
    }
    else
    {
        return makeResult(false, "That action is not used in this mission.");
    }

    if (changed)
    {
        m_undo.push(mission == MissionKind::Reconstruct ? before : std::string("mirror"));
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
            break;
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
        built = buildAutoTree(level);
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
        LevelDef g = level;

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
            return false;
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
            m_rankStart = n - m_k + 1;
            m_rankLeft = m_rankStart;

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

void GameSession::resetMissionState()
{
    m_found.clear();
    m_cursor = kNull;
    m_rankLeft = m_rankStart;
    m_queuePos = 0;
    m_rangeFound.clear();
}

void GameSession::restoreInitial()
{
    if (m_isBinary)
    {
        m_built.clear();
        buildBinaryTree(m_level, m_bin);
        return;
    }

    if (m_isHeap)
    {
        restoreHeap(m_initialHeap);
        m_opsUsed = 0;
        return;
    }

    if (m_isAvl)
    {
        Serializer::deserializeAVL(m_initialTree, m_avl);
        resetMissionState();
        return;
    }

    m_rangeFound.clear();
    Serializer::deserializeBST(m_initialTree, m_tree);
}

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
        bool restored = false;

        if (m_isBinary)
        {
            if (m_level.mission == MissionKind::Reconstruct)
            {
                restored = Serializer::deserializeBinaryTree(previous, m_built);
            }
            else
            {
                m_bin.mirror(nullptr);
                restored = true;
            }
        }

        else if (m_isHeap)
        {
            restored = restoreHeap(previous);
        }

        else if (m_isAvl)
        {
            restored = restoreAvl(previous);
        }
        else
        {
            restored = Serializer::deserializeBST(previous, m_tree);
        }

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

        if (m_isHeap)
        {
            text = heapHint();
        }

        else if (m_isAvl)
        {
            text = avlHint();
        }

        else if (isReconstruct())
        {
            text = reconstructHint();
        }

        else if (m_level.mission == MissionKind::BstRange)
        {
            text = "Collect every key with lo <= key <= hi. A subtree whose keys all lie outside the range is skipped. (Star cap: 2)";
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

    if (m_isBinary)
    {
        return applyBinary(action);
    }

    if (m_isHeap)
    {
        return applyHeap(action);
    }

    if (m_isAvl)
    {
        return applyAvl(action);
    }

    if (m_level.mission == MissionKind::BstRange)
    {
        return applyRange(action);
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
                m_avl.rotateLeft(child, &m_lastLog);

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

        m_avl.insert(m_queue[m_queuePos], &m_lastLog);
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

ActionResult GameSession::applyRange(const PlayerAction& action)
{
    m_lastLog.clear();
    m_lastLog.beginBatch();

    if (action.kind != ActionKind::ClickNode)
    {
        return makeResult(false, "Click the keys inside the range.");
    }

    const BSTNode* node = nullptr;
    const NodePtrArray& pool = m_tree.pool();

    if (action.a >= 0 && action.a < pool.size() && m_tree.isAlive(action.a))
    {
        node = static_cast<const BSTNode*>(pool[action.a]);
    }

    if (node == nullptr)
    {
        return makeResult(false, "Click a room.");
    }

    Event visit(EventType::VISIT);
    visit.id = node->id;
    visit.alarmCost = 1;
    emit(&m_lastLog, visit);

    bool accepted = false;
    bool completed = false;
    int extraAlarm = 0;
    int penalty = 0;
    std::string message;

    const bool inRange = node->key >= m_rangeLo && node->key <= m_rangeHi;
    const bool alreadyCollected = m_rangeFound.indexOf(node->key) != kNull;

    if (alreadyCollected)
    {
        extraAlarm = 1;
        penalty = kWrongClickPenalty;

        message = "Key " + std::to_string(node->key) + " is already collected. +1 alarm";
    }
    else if (!inRange)
    {
        extraAlarm = 1;
        penalty = kWrongClickPenalty;

        message = "Key " + std::to_string(node->key) + " is outside [" + std::to_string(m_rangeLo) + ", " +
                  std::to_string(m_rangeHi) + "]. +1 alarm";
    }
    else
    {
        m_rangeFound.push(node->key);
        accepted = true;
        completed = (m_rangeFound.size() == m_rangeExpected.size());

        message = "Collected " + std::to_string(node->key) + " (" + std::to_string(m_rangeFound.size()) +
                  " of " + std::to_string(m_rangeExpected.size()) + ").";
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

int GameSession::rangeLo() const
{
    return m_level.mission == MissionKind::BstRange ? m_rangeLo : kNull;
}

int GameSession::rangeHi() const
{
    return m_level.mission == MissionKind::BstRange ? m_rangeHi : kNull;
}

int GameSession::rangeOptimal() const
{
    return m_level.mission == MissionKind::BstRange ? m_rangeOptimal : 0;
}

std::string GameSession::rangePrompt() const
{
    return "Collect every key in [" + std::to_string(m_rangeLo) + ", " + std::to_string(m_rangeHi) + "]: " +
           std::to_string(m_rangeFound.size()) + " of " + std::to_string(m_rangeExpected.size()) + " collected.";
}

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
        return kNull;
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

    if (m_isHeap)
    {
        return heapPrompt();
    }

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

    if (mission == MissionKind::BstRange)
    {
        return rangePrompt();
    }

    if (m_target != kNull)
    {
        return "Target key: " + std::to_string(m_target);
    }

    return "";
}

ViewSnapshot GameSession::snapshot() const
{
    if (isReconstruct())
    {
        return SnapshotBuilder::fromBinaryTree(m_built);
    }

    if (m_isHeap)
    {
        return SnapshotBuilder::fromMaxHeap(m_heap);
    }

    if (m_isBinary)
    {
        return SnapshotBuilder::fromBinaryTree(m_bin);
    }

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

const BinaryTree& GameSession::binaryTree() const
{
    return m_bin;
}

int GameSession::targetKey() const
{
    return (m_isAvl || m_isBinary) ? kNull : m_target;
}

int GameSession::lcaTargetA() const
{
    return m_isBinary ? m_lcaA : kNull;
}

int GameSession::lcaTargetB() const
{
    return m_isBinary ? m_lcaB : kNull;
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
    if (isReconstruct())
    {
        int v = 0;
        int p = kNull;
        int s = Side::None;
        return nextPlacement(v, p, s) ? p : kNull;
    }

    if (m_isHeap)
    {
        return heapTargetIndex();
    }

    if (m_isAvl)
    {
        return avlTargetNode();
    }

    if (m_isBinary || m_target == kNull || m_level.mission == MissionKind::BstInsert)
    {
        return kNull;
    }

    BSTNode* node = m_tree.search(m_target, nullptr);
    return node == nullptr ? kNull : node->id;
}

// ============================================================================
// Act 4: Max Heap missions (4.1 to 4.5)
// ============================================================================

namespace
{
    int countEventType(const EventLog& log, int type)
    {
        int count = 0;
        for (int i = 0; i < log.size(); i++)
        {
            if (log.at(i).type == type)
            {
                count++;
            }
        }
        return count;
    }

    bool isParentOf(int parent, int child)
    {
        return child > 0 && (child - 1) / 2 == parent;
    }
}

bool GameSession::isHeapMission(int mission)
{
    return mission >= MissionKind::HeapBuild && mission <= MissionKind::HeapBoss;
}

const MaxHeap& GameSession::heap() const
{
    return m_heap;
}

int GameSession::opsUsed() const
{
    return m_opsUsed;
}

int GameSession::opLimit() const
{
    return m_opLimit;
}

bool GameSession::startHeap(const LevelDef& level)
{
    int n = level.nodeCount;

    if (n < 3 || n > 31)
    {
        n = 12;
    }

    std::uint32_t state = static_cast<std::uint32_t>(level.seed) * 2654435761u + 11u;
    IntArray values;
    for (int i = 0; i < n; i++)
    {
        int value = 0;

        if (level.style == 2)
        {
            value = (i + 1) * 3;
        }

        else if (level.style == 3)
        {
            value = (n - i) * 3;
        }

        else if (level.style == 5)
        {
            value = static_cast<int>(nextRandom(state) % 5u) + 1;
        }

        else
        {
            value = static_cast<int>(nextRandom(state) % 90u) + 10;
        }

        values.push(value);
    }

    const int mission = level.mission;

    if (mission == MissionKind::HeapExtract || mission == MissionKind::HeapRepair)
    {
        m_heap.buildMaxHeap(values);
    }
    else
    {
        m_heap.loadRaw(values, n);
    }

    if (mission == MissionKind::HeapRepair)
    {
        IntArray disturbed = m_heap.array();
        disturbed[0] = 0;
        m_heap.loadRaw(disturbed, n);
    }

    m_opLimit = (mission == MissionKind::HeapRace && level.param > 0) ? level.param : 0;
    m_initialHeap = captureHeap();

    MaxHeap scratch;
    scratch.loadRaw(m_heap.array(), m_heap.activeSize());
    EventLog canonical;
    int optimal = 1;

    if (mission == MissionKind::HeapBuild)
    {
        scratch.buildMaxHeap(&canonical);
        optimal = countEventType(canonical, EventType::HEAPIFY_START);
    }

    else if (mission == MissionKind::HeapRepair)
    {
        scratch.siftDown(0, scratch.activeSize(), &canonical);
        optimal = countEventType(canonical, EventType::HEAPIFY_START);
    }

    else if (mission == MissionKind::HeapExtract)
    {
        int ignored = 0;

        while (scratch.activeSize() > 0)
        {
            scratch.extractMax(ignored, &canonical);
        }

        optimal = countEventType(canonical, EventType::EXTRACT_MAX);
    }

    else
    {
        scratch.heapSort(&canonical);
        optimal = 1 + countEventType(canonical, EventType::EXTRACT_MAX);
    }

    if (optimal < 1)
    {
        optimal = 1;
    }

    m_score.configure(optimal, level.objective);
    m_started = true;
    return true;
}

std::string GameSession::captureHeap() const
{
    const IntArray& values = m_heap.array();
    std::ostringstream out;

    out << m_heap.activeSize() << ' ' << values.size();

    for (int i = 0; i < values.size(); i++)
    {
        out << ' ' << values[i];
    }

    return out.str();
}

bool GameSession::restoreHeap(const std::string& text)
{
    std::istringstream in(text);
    int active = 0;
    int total = 0;

    if (!(in >> active >> total) || total < 0 || active < 0 || active > total)
    {
        return false;
    }

    IntArray values;

    for (int i = 0; i < total; i++)
    {
        int value = 0;
        if (!(in >> value))
        {
            return false;
        }
        values.push(value);
    }

    return m_heap.loadRaw(values, active);
}

int GameSession::heapTargetIndex() const
{
    const int active = m_heap.activeSize();
    const IntArray& a = m_heap.array();

    for (int i = 0; i < active; i++)
    {
        const int l = 2 * i + 1;
        const int r = l + 1;

        if ((l < active && a[l] > a[i]) || (r < active && a[r] > a[i]))
        {
            return i;
        }
    }

    const bool extracting = m_level.mission == MissionKind::HeapExtract ||
                            m_level.mission == MissionKind::HeapRace || m_level.mission == MissionKind::HeapBoss;
    if (extracting && active > 0)
    {
        return 0;
    }

    return kNull;
}

std::string GameSession::heapHint() const
{
    const int target = heapTargetIndex();

    if (target == kNull)
    {
        return "Nothing to fix here: pick the next step. (Star cap: 2)";
    }

    const int active = m_heap.activeSize();
    const IntArray& a = m_heap.array();
    const int l = 2 * target + 1;
    const int r = l + 1;
    const bool violated = (l < active && a[l] > a[target]) || (r < active && a[r] > a[target]);

    if (violated)
    {
        return "Index " + std::to_string(target) + " is smaller than a child. Swap it with the larger child, or Heapify " +
               std::to_string(target) + ". (Star cap: 2)";
    }

    return "The root holds the greatest active value. Extract it. (Star cap: 2)";
}

std::string GameSession::heapPrompt() const
{
    const int total = m_heap.totalSize();
    const int active = m_heap.activeSize();

    std::string text = "Active heap: " + std::to_string(active) + " of " + std::to_string(total) +
                       " cells. Sorted region: " + std::to_string(total - active) + ".";
    if (m_opLimit > 0)
    {
        text += " Operations: " + std::to_string(m_opsUsed) + " / " + std::to_string(m_opLimit) + ".";
    }

    return text;
}

ActionResult GameSession::applyHeap(const PlayerAction& action)
{
    m_lastLog.clear();
    m_lastLog.beginBatch();
    const int mission = m_level.mission;

    const bool buildsAndSorts = mission == MissionKind::HeapRace || mission == MissionKind::HeapBoss;
    const bool canSwapAndHeapify = mission == MissionKind::HeapBuild ||
                                   mission == MissionKind::HeapRepair || buildsAndSorts;
    const bool canExtract = mission == MissionKind::HeapExtract || buildsAndSorts;

    if (action.kind == ActionKind::HeapSort)
    {
        return makeResult(false, "Heap Sort is not one click here: build the heap and extract each maximum yourself.");
    }

    bool allowed = false;

    if (action.kind == ActionKind::SwapHeap || action.kind == ActionKind::Heapify)
    {
        allowed = canSwapAndHeapify;
    }

    else if (action.kind == ActionKind::ExtractMax)
    {
        allowed = canExtract;
    }

    else if (action.kind == ActionKind::BuildHeap)
    {
        allowed = buildsAndSorts;
    }

    if (!allowed)
    {
        return makeResult(false, "That action is not used in this mission.");
    }

    const std::string before = captureHeap();
    const int active = m_heap.activeSize();
    const IntArray& a = m_heap.array();
    m_opsUsed++;

    bool accepted = false;
    bool changed = false;
    int stepCost = 1;
    int penalty = 0;
    std::string message;

    if (action.kind == ActionKind::SwapHeap)
    {
        const int i = action.a;
        const int j = action.b;
        int parent = kNull;
        int child = kNull;
        if (i >= 0 && j >= 0 && i < active && j < active)
        {
            if (isParentOf(i, j))
            {
                parent = i;
                child = j;
            }
            else if (isParentOf(j, i))
            {
                parent = j;
                child = i;
            }
        }

        if (parent == kNull)
        {
            stepCost = 2;
            penalty = kWrongClickPenalty;
            message = "Pick a parent and one of its children inside the active heap. +1 alarm";
        }
        else if (a[child] <= a[parent])
        {
            stepCost = 2;
            penalty = kWrongClickPenalty;
            message = "That swap breaks the heap: only a larger child can rise above its parent. +1 alarm";
        }
        else
        {
            m_heap.swapIdx(parent, child, &m_lastLog);
            accepted = true;
            changed = true;
            message = "Swapped " + std::to_string(parent) + " and " + std::to_string(child) + ".";
        }
    }
    else if (action.kind == ActionKind::Heapify)
    {
        if (action.a < 0 || action.a >= active)
        {
            stepCost = 2;
            penalty = kWrongClickPenalty;
            message = "That index is outside the active heap. +1 alarm";
        }
        else
        {
            m_heap.siftDown(action.a, active, &m_lastLog);
            accepted = true;
            changed = true;
            message = "Heapified from index " + std::to_string(action.a) + ".";
        }
    }
    else if (action.kind == ActionKind::BuildHeap)
    {
        if (active != m_heap.totalSize())
        {
            stepCost = 2;
            penalty = kWrongClickPenalty;
            message = "Build only before the first extraction. +1 alarm";
        }
        else
        {
            m_heap.buildMaxHeap(&m_lastLog);
            accepted = true;
            changed = true;
            message = "Bottom-up heap construction over the whole array.";
        }
    }
    else if (action.kind == ActionKind::ExtractMax)
    {
        if (active == 0)
        {
            stepCost = 2;
            penalty = kWrongClickPenalty;
            message = "Nothing is left in the active heap. +1 alarm";
        }
        else if (!m_heap.isMaxHeap())
        {
            stepCost = 2;
            penalty = kWrongClickPenalty;
            message = "The heap property is broken: repair it before extracting. +1 alarm";
        }
        else
        {
            int value = 0;
            m_heap.extractMax(value, &m_lastLog);
            accepted = true;
            changed = true;
            message = "Extracted maximum " + std::to_string(value) + " into the sorted region.";
        }
    }

    if (changed)
    {
        m_undo.push(before);
    }

    m_score.recordAction(stepCost, penalty);

    if (m_score.status() == HudStatus::Playing)
    {
        if (LevelManager::heapWon(mission, m_heap))
        {
            m_score.markWon();
        }
        else if (m_opLimit > 0 && m_opsUsed >= m_opLimit)
        {
            m_score.markLost();
            message += " Operation limit reached.";
        }
    }

    Event done(EventType::DONE);
    done.key = stepCost;
    emit(&m_lastLog, done);
    return makeResult(accepted, message);
}

const BinaryTree& GameSession::builtTree() const
{
    return m_built;
}

bool GameSession::isReconstruct() const
{
    return m_isBinary && m_level.mission == MissionKind::Reconstruct;
}

IntArray GameSession::reconstructTokens() const
{
    IntArray out;

    if (!isReconstruct())
    {
        return out;
    }

    const NodePtrArray& pool = m_bin.pool();

    for (int i = 0; i < pool.size(); i++)
    {
        if (m_bin.isAlive(i) && findValueId(m_built, pool[i]->value) == kNull)
        {
            out.push(pool[i]->value);
        }
    }

    for (int i = 1; i < out.size(); i++)
    {
        const int v = out[i];
        int j = i;

        while (j > 0 && out[j - 1] > v)
        {
            out[j] = out[j - 1];
            j--;
        }

        out[j] = v;
    }

    return out;
}

std::string GameSession::reconstructClue() const
{
    if (!isReconstruct())
    {
        return "";
    }

    const bool post = (m_level.param == 1);
    const IntArray first = post ? m_bin.postorder() : m_bin.preorder();
    const IntArray in = m_bin.inorder();
    std::string s = post ? "Postorder:" : "Preorder:";

    for (int i = 0; i < first.size(); i++)
    {
        s += " " + std::to_string(m_bin.pool()[first[i]]->value);
    }

    s += "   Inorder:";

    for (int i = 0; i < in.size(); i++)
    {
        s += " " + std::to_string(m_bin.pool()[in[i]]->value);
    }

    return s;
}

bool GameSession::nextPlacement(int& value, int& parentId, int& side) const
{
    const IntArray order = m_bin.levelOrder(nullptr);

    for (int i = 0; i < order.size(); i++)
    {
        const TreeNode* h = m_bin.pool()[order[i]];

        if (findValueId(m_built, h->value) != kNull)
        {
            continue;
        }

        value = h->value;

        if (h->parent == nullptr)
        {
            parentId = kNull;
            side = Side::None;
            return true;
        }

        parentId = findValueId(m_built, h->parent->value);
        side = (h->parent->left == h) ? Side::Left : Side::Right;
        return parentId != kNull;
    }

    return false;
}

std::string GameSession::reconstructHint() const
{
    int v = 0;
    int p = kNull;
    int s = Side::None;

    if (!nextPlacement(v, p, s))
    {
        return "Every token is placed.";
    }

    if (p == kNull)
    {
        return "Place token " + std::to_string(v) + " as the ROOT. (Star cap: 2)";
    }

    return "Place token " + std::to_string(v) + " on the " + (s == Side::Left ? "left" : "right") +
           " of room " + std::to_string(m_built.pool()[p]->value) + ". (Star cap: 2)";
}