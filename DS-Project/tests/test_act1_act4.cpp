// tests/test_act1_act4.cpp
// Playthrough tests. Each playable campaign level is solved through GameSession::apply()
// using answers taken from the model (what a correct player can read from the prompt and
// the hint). Every level must end in HudStatus::Won.
// Level 2.4 (range query) is not playable yet and is skipped.
#include "TestSupport.h"
#include "../game/GameSession.h"
#include "../game/LevelLoader.h"
#include "../model/Serializer.h"

namespace
{
    // ------------------------------------------------------------ helpers

    PlayerAction makeAction(int kind, int a = kNull, int b = kNull, int c = kNull)
    {
        PlayerAction p;
        p.kind = kind;
        p.a = a;
        p.b = b;
        p.c = c;
        return p;
    }

    bool loadCampaignLevel(int act, int slot, LevelDef& out)
    {
        LevelDefArray defs;
        std::string error;
        const std::string path = std::string("DS-Project/levels/act") + static_cast<char>('0' + act) + ".json";
        if (!LevelLoader::loadFile(path, defs, &error) || defs.size() != 5)
        {
            return false;
        }
        out = defs[slot];
        return true;
    }

    bool startCampaignLevel(GameSession& s, int act, int slot)
    {
        LevelDef def;
        return loadCampaignLevel(act, slot, def) && s.startLevel(def);
    }

    bool playing(const GameSession& s)
    {
        return s.hud().status == HudStatus::Playing;
    }

    bool won(const GameSession& s)
    {
        return s.hud().status == HudStatus::Won;
    }

    IntArray traversalFor(const BinaryTree& t, int type)
    {
        if (type == 1)
        {
            return t.inorder();
        }
        if (type == 2)
        {
            return t.postorder();
        }
        if (type == 3)
        {
            return t.levelOrder();
        }
        return t.preorder();
    }

    IntArray reversedOf(const IntArray& in)
    {
        IntArray out;
        for (int i = in.size() - 1; i >= 0; i--)
        {
            out.push(in[i]);
        }
        return out;
    }

    int subtreeSize(const TreeNode* n)
    {
        return n == nullptr ? 0 : static_cast<const AVLNode*>(n)->size;
    }

    // First alive AVL node whose balance factor is outside [-1, 1], or kNull.
    int violatedRoom(const AVLTree& t)
    {
        const NodePtrArray& pool = t.pool();
        for (int i = 0; i < pool.size(); i++)
        {
            if (!t.isAlive(i))
            {
                continue;
            }
            const int bf = AVLTree::balanceFactor(static_cast<const AVLNode*>(pool[i]));
            if (bf > 1 || bf < -1)
            {
                return i;
            }
        }
        return kNull;
    }

    // Textbook rotation for a violated room (same rule the model uses).
    int correctRotation(const AVLNode* n)
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

    // Rotation the next insert will cause, found on a scratch copy of the tree.
    int predictRotation(const AVLTree& tree, int key)
    {
        AVLTree scratch;
        Serializer::deserializeAVL(Serializer::serializeAVL(tree), scratch);
        EventLog log;
        scratch.insert(key, &log);
        for (int i = 0; i < log.size(); i++)
        {
            if (log.at(i).type == EventType::ROTATE)
            {
                return log.at(i).result;
            }
        }
        return RotationType::None;
    }

    int findBuiltId(const BinaryTree& built, int value)
    {
        const NodePtrArray& pool = built.pool();
        for (int i = 0; i < pool.size(); i++)
        {
            if (built.isAlive(i) && pool[i]->value == value)
            {
                return i;
            }
        }
        return kNull;
    }

    // Places every hidden value in level order: parents always come before children.
    bool solveReconstruct(GameSession& s)
    {
        const BinaryTree& hidden = s.binaryTree();
        const IntArray order = hidden.levelOrder(nullptr);
        for (int i = 0; i < order.size() && playing(s); i++)
        {
            const TreeNode* h = hidden.pool()[order[i]];
            PlayerAction p = makeAction(ActionKind::Place, h->value, kNull, Side::None);
            if (h->parent != nullptr)
            {
                p.b = findBuiltId(s.builtTree(), h->parent->value);
                p.c = (h->parent->left == h) ? Side::Left : Side::Right;
            }
            if (!s.apply(p).accepted)
            {
                return false;
            }
        }
        return won(s);
    }

    // Finds a parent with a child that is not larger. Swapping them must be rejected.
    bool findWrongSwap(const MaxHeap& heap, int& parent, int& child)
    {
        const IntArray& a = heap.array();
        const int n = heap.activeSize();
        for (int p = 0; p < n; p++)
        {
            for (int k = 1; k <= 2; k++)
            {
                const int c = 2 * p + k;
                if (c < n && a[c] <= a[p])
                {
                    parent = p;
                    child = c;
                    return true;
                }
            }
        }
        return false;
    }

    bool extractAll(GameSession& s)
    {
        int guard = 0;
        while (playing(s) && s.heap().activeSize() > 0 && guard++ < 64)
        {
            if (!s.apply(makeAction(ActionKind::ExtractMax)).accepted)
            {
                return false;
            }
        }
        return won(s);
    }

    // ------------------------------------------------------------ Act 1

    void playAct1()
    {
        // 1.1 traversal: reversed preorder must fail, the real preorder must win.
        {
            GameSession s;
            LevelDef def;
            testCheck(loadCampaignLevel(1, 0, def) && s.startLevel(def), "1.1 starts");
            const IntArray order = traversalFor(s.binaryTree(), def.param);

            PlayerAction wrong = makeAction(ActionKind::SubmitSequence);
            wrong.seq = reversedOf(order);
            const ActionResult bad = s.apply(wrong);
            testCheck(!bad.accepted && playing(s) && bad.hud.penalties == 10, "1.1 reversed order is rejected");

            PlayerAction right = makeAction(ActionKind::SubmitSequence);
            right.seq = order;
            testCheck(s.apply(right).accepted && won(s), "1.1 traversal solved");
        }

        // 1.2 reconstruction, plus the shelf that reconstructTokens() reports.
        {
            GameSession s;
            testCheck(startCampaignLevel(s, 1, 1), "1.2 starts");
            const IntArray shelf = s.reconstructTokens();
            testCheck(shelf.size() == s.binaryTree().nodeCount(), "1.2 every token starts on the shelf");

            bool ascending = true;
            for (int i = 1; i < shelf.size(); i++)
            {
                if (shelf[i - 1] > shelf[i])
                {
                    ascending = false;
                }
            }
            testCheck(ascending, "1.2 shelf is ascending");

            const int rootValue = s.binaryTree().root()->value;
            s.apply(makeAction(ActionKind::Place, rootValue, kNull, Side::None));
            const IntArray after = s.reconstructTokens();
            testCheck(after.size() == shelf.size() - 1 && after.indexOf(rootValue) == kNull,
                "1.2 placed token leaves the shelf");

            GameSession fresh;
            testCheck(startCampaignLevel(fresh, 1, 1) && solveReconstruct(fresh), "1.2 tree rebuilt");
        }

        // 1.3 height and diameter.
        {
            GameSession s;
            testCheck(startCampaignLevel(s, 1, 2), "1.3 starts");
            const int height = s.binaryTree().height();
            const int diameter = s.binaryTree().diameter();
            testCheck(!s.apply(makeAction(ActionKind::SubmitSequence, height + 1, diameter)).accepted && playing(s),
                "1.3 wrong height is rejected");
            testCheck(s.apply(makeAction(ActionKind::SubmitSequence, height, diameter)).accepted && won(s),
                "1.3 height and diameter solved");
        }

        // 1.4 lowest common ancestor of rooms n-1 and n/2.
        {
            GameSession s;
            testCheck(startCampaignLevel(s, 1, 3), "1.4 starts");
            const int n = s.binaryTree().nodeCount();
            const int common = s.binaryTree().lca(n - 1, n / 2, nullptr);
            testCheck(common != kNull, "1.4 lca exists");
            const int wrongId = (common == 0) ? 1 : 0;
            testCheck(!s.apply(makeAction(ActionKind::ClickNode, wrongId)).accepted && playing(s),
                "1.4 wrong room is rejected");
            testCheck(s.apply(makeAction(ActionKind::ClickNode, common)).accepted && won(s), "1.4 meeting room solved");
        }

        // 1.5 mirror.
        {
            GameSession s;
            testCheck(startCampaignLevel(s, 1, 4), "1.5 starts");
            testCheck(s.apply(makeAction(ActionKind::Rotate)).accepted && won(s), "1.5 mirror solved");
        }
    }

    // ------------------------------------------------------------ Act 2

    void playAct2()
    {
        // 2.1 search.
        {
            GameSession s;
            testCheck(startCampaignLevel(s, 2, 0), "2.1 starts");
            const int target = s.targetKey();
            testCheck(target != kNull, "2.1 has a target");
            testCheck(s.apply(makeAction(ActionKind::Search, target)).accepted && won(s), "2.1 target found");
        }

        // 2.2 insert.
        {
            GameSession s;
            testCheck(startCampaignLevel(s, 2, 1), "2.2 starts");
            const int target = s.targetKey();
            testCheck(s.tree().search(target, nullptr) == nullptr, "2.2 target is not yet in the tree");
            testCheck(s.apply(makeAction(ActionKind::Insert, target)).accepted && won(s), "2.2 target inserted");
        }

        // 2.3 delete: the player names the delete case first.
        {
            GameSession s;
            testCheck(startCampaignLevel(s, 2, 2), "2.3 starts");
            const int target = s.targetKey();
            const BSTNode* node = s.tree().search(target, nullptr);
            testCheck(node != nullptr, "2.3 target exists");

            int correct = DeleteCase::OneChild;
            if (node->left != nullptr && node->right != nullptr)
            {
                correct = DeleteCase::TwoChildren;
            }
            else if (node->left == nullptr && node->right == nullptr)
            {
                correct = DeleteCase::Leaf;
            }
            const int wrong = (correct + 1) % 3;

            const ActionResult bad = s.apply(makeAction(ActionKind::Delete, target, kNull, wrong));
            testCheck(!bad.accepted && bad.hud.penalties == 10 && playing(s), "2.3 wrong delete case is rejected");
            testCheck(s.apply(makeAction(ActionKind::Delete, target, kNull, correct)).accepted && won(s),
                "2.3 delete solved");
        }

        // 2.4 range raid: click every key in [lo, hi]. A key outside the range is a wrong click.
        {
            GameSession s;
            testCheck(startCampaignLevel(s, 2, 3), "2.4 starts");
            testCheck(s.rangeLo() != kNull && s.rangeHi() != kNull, "2.4 exposes its bounds");

            // The first key outside the range is a wrong click that costs 10 points.
            const NodePtrArray& pool = s.tree().pool();
            int outsideId = kNull;
            int insideId = kNull;
            for (int i = 0; i < pool.size(); i++)
            {
                if (!s.tree().isAlive(i))
                {
                    continue;
                }
                const int key = static_cast<const BSTNode*>(pool[i])->key;
                if (outsideId == kNull && (key < s.rangeLo() || key > s.rangeHi()))
                {
                    outsideId = i;
                }
            }
            if (outsideId != kNull)
            {
                const ActionResult bad = s.apply(makeAction(ActionKind::ClickNode, outsideId));
                testCheck(!bad.accepted && bad.hud.penalties == 10 && playing(s), "2.4 key outside the range is rejected");
            }

            // Collect every in-range key. Keys are in ascending order for the check below.
            const IntArray expected = s.tree().rangeQuery(s.rangeLo(), s.rangeHi(), nullptr);
            for (int i = 0; i < expected.size() && playing(s); i++)
            {
                const BSTNode* node = s.tree().search(expected[i], nullptr);
                testCheck(node != nullptr, "2.4 expected key exists");
                testCheck(s.apply(makeAction(ActionKind::ClickNode, node->id)).accepted, "2.4 in-range key accepted");
            }
            testCheck(won(s), "2.4 range raid solved");
            testCheck(s.rangeOptimal() >= 1, "2.4 optimal steps come from the model");
            (void)insideId;
        }

        // 2.5 boss: search the degenerate chain for its deepest key.
        {
            GameSession s;
            testCheck(startCampaignLevel(s, 2, 4), "2.5 starts");
            const int target = s.targetKey();
            testCheck(s.apply(makeAction(ActionKind::Search, target)).accepted && won(s), "2.5 boss solved");
        }
    }

    // ------------------------------------------------------------ Act 3

    void playAvlRepair(int slot, const char* name)
    {
        GameSession s;
        testCheck(startCampaignLevel(s, 3, slot), name);
        const int pivot = violatedRoom(s.avlTree());
        testCheck(pivot != kNull, "repair level starts unbalanced");

        const AVLNode* node = static_cast<const AVLNode*>(s.avlTree().pool()[pivot]);
        const int correct = correctRotation(node);
        testCheck(correct != RotationType::None, "repair level has a correct rotation");

        const int wrong = (correct == RotationType::LL) ? RotationType::RR
            : (correct == RotationType::RR) ? RotationType::LL
            : (correct == RotationType::LR) ? RotationType::RL
            : RotationType::LR;

        const ActionResult bad = s.apply(makeAction(ActionKind::Rotate, pivot, wrong));
        testCheck(!bad.accepted && bad.hud.penalties == 50 && playing(s), "wrong rotation costs 50 points");
        testCheck(s.apply(makeAction(ActionKind::Rotate, pivot, correct)).accepted && won(s), "repair solved");
    }

    void playAct3()
    {
        // 3.1 badges: click every unbalanced room.
        {
            GameSession s;
            testCheck(startCampaignLevel(s, 3, 0), "3.1 starts");
            const NodePtrArray& pool = s.avlTree().pool();
            int clicked = 0;
            for (int i = 0; i < pool.size() && playing(s); i++)
            {
                if (!s.avlTree().isAlive(i))
                {
                    continue;
                }
                const int bf = AVLTree::balanceFactor(static_cast<const AVLNode*>(pool[i]));
                if (bf > 1 || bf < -1)
                {
                    clicked++;
                    testCheck(s.apply(makeAction(ActionKind::ClickNode, i)).accepted, "3.1 unbalanced room accepted");
                }
            }
            testCheck(clicked > 0, "3.1 has unbalanced rooms");
            testCheck(won(s), "3.1 all unbalanced rooms marked");
        }

        // 3.2 single rotation and 3.3 double rotation.
        playAvlRepair(1, "3.2 starts");
        playAvlRepair(2, "3.3 starts");

        // 3.4 auto insertion: predict each rotation before it happens.
        {
            GameSession s;
            testCheck(startCampaignLevel(s, 3, 3), "3.4 starts");
            bool wrongFirst = true;
            int guard = 0;
            while (playing(s) && s.nextInsertKey() != kNull && guard++ < 50)
            {
                const int predicted = predictRotation(s.avlTree(), s.nextInsertKey());
                int answer = predicted;
                if (wrongFirst)
                {
                    answer = (predicted == RotationType::None) ? RotationType::LL : RotationType::None;
                    wrongFirst = false;
                }
                testCheck(s.apply(makeAction(ActionKind::Rotate, kNull, answer)).accepted, "3.4 prediction accepted");
            }
            testCheck(won(s), "3.4 all inserts predicted");
            testCheck(s.hud().penalties == 50, "3.4 the one wrong prediction cost 50 points");
        }

        // 3.5 order statistic: walk from the root with the rank rule.
        {
            GameSession s;
            testCheck(startCampaignLevel(s, 3, 4), "3.5 starts");
            const AVLTree& t = s.avlTree();
            int rank = t.nodeCount() - s.orderK() + 1;
            const AVLNode* cur = static_cast<const AVLNode*>(t.root());
            while (cur != nullptr && playing(s))
            {
                testCheck(s.apply(makeAction(ActionKind::ClickNode, cur->id)).accepted, "3.5 path room accepted");
                const int left = subtreeSize(cur->left);
                if (rank == left + 1)
                {
                    break;
                }
                if (rank <= left)
                {
                    cur = static_cast<const AVLNode*>(cur->left);
                }
                else
                {
                    rank -= left + 1;
                    cur = static_cast<const AVLNode*>(cur->right);
                }
            }
            testCheck(won(s), "3.5 order statistic found");
        }
    }

    // ------------------------------------------------------------ Act 4

    void playAct4()
    {
        // 4.1 bottom-up construction. A parent-child swap that breaks the heap is rejected.
        {
            GameSession s;
            testCheck(startCampaignLevel(s, 4, 0), "4.1 starts");

            int p = kNull;
            int c = kNull;
            if (findWrongSwap(s.heap(), p, c))
            {
                const ActionResult bad = s.apply(makeAction(ActionKind::SwapHeap, p, c));
                testCheck(!bad.accepted && bad.hud.penalties == 10 && playing(s), "4.1 breaking swap is rejected");
            }

            const int n = s.heap().totalSize();
            for (int i = n / 2 - 1; i >= 0 && playing(s); i--)
            {
                testCheck(s.apply(makeAction(ActionKind::Heapify, i)).accepted, "4.1 heapify accepted");
            }
            testCheck(won(s) && s.heap().isMaxHeap(), "4.1 max heap built");
        }

        // 4.2 repair the overwritten root.
        {
            GameSession s;
            testCheck(startCampaignLevel(s, 4, 1), "4.2 starts");
            testCheck(s.heap().array()[0] == 0, "4.2 root is disturbed");
            testCheck(s.apply(makeAction(ActionKind::Heapify, 0)).accepted && won(s), "4.2 heap property restored");
        }

        // 4.3 extract max, with undo.
        {
            GameSession s;
            testCheck(startCampaignLevel(s, 4, 2), "4.3 starts");
            const int before = s.heap().activeSize();
            s.apply(makeAction(ActionKind::ExtractMax));
            s.apply(makeAction(ActionKind::Undo));
            testCheck(s.heap().activeSize() == before, "4.3 undo restores the heap size");
            testCheck(extractAll(s), "4.3 every maximum extracted");
            testCheck(s.heap().isSortedAscending(), "4.3 sorted region is ascending");
        }

        // 4.4 race: build plus extractions must fit the operation limit.
        {
            GameSession s;
            testCheck(startCampaignLevel(s, 4, 3), "4.4 starts");
            testCheck(s.apply(makeAction(ActionKind::BuildHeap)).accepted, "4.4 build accepted");
            testCheck(extractAll(s), "4.4 race won");
            testCheck(s.opsUsed() <= s.opLimit(), "4.4 finished within the operation limit");
        }

        // 4.5 boss: one-click heap sort is refused, the manual route wins.
        {
            GameSession s;
            testCheck(startCampaignLevel(s, 4, 4), "4.5 starts");
            testCheck(!s.apply(makeAction(ActionKind::HeapSort)).accepted, "4.5 one-click heap sort is refused");
            testCheck(s.apply(makeAction(ActionKind::BuildHeap)).accepted, "4.5 build accepted");
            testCheck(extractAll(s), "4.5 final vault opened");
            testCheck(s.heap().activeSize() == 0 && s.heap().isSortedAscending(), "4.5 array sorted ascending");
        }
    }
}

// ------------------------------------------------------------ entry points

void runAct1ToAct3Playthroughs()
{
    playAct1();
    playAct2();
    playAct3();
}

void runAct4SessionTests()
{
    playAct4();
}

void runCampaignConstraintTests()
{
    // Act 1: 6 to 15 rooms.
    for (int slot = 0; slot < 5; slot++)
    {
        LevelDef def;
        testCheck(loadCampaignLevel(1, slot, def) && def.nodeCount >= 6 && def.nodeCount <= 15,
            "act 1 level sizes are 6 to 15 rooms");
    }

    // 4.4: the model's optimum must fit inside the operation limit.
    GameSession race;
    testCheck(startCampaignLevel(race, 4, 3), "4.4 starts for the constraint check");
    testCheck(race.opLimit() > 0 && race.hud().optimalSteps <= race.opLimit(),
        "4.4 optimal steps fit inside the operation limit");

    // Every Act 4 level must be a heap mission.
    for (int slot = 0; slot < 5; slot++)
    {
        LevelDef def;
        testCheck(loadCampaignLevel(4, slot, def) && GameSession::isHeapMission(def.mission),
            "act 4 levels are heap missions");
    }
}