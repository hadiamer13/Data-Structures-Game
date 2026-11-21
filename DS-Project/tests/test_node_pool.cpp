#include "TestSupport.h"
#include "../model/NodePool.h"
#include "../model/TreeNode.h"
#include "../model/EventLog.h"

void runNodePoolTests()
{
    NodePool pool;
    TreeNode* a = pool.createTree(1);
    BSTNode* b = pool.createBST(2);
    AVLNode* c = pool.createAVL(3);
    testCheck(a->id == 0 && b->id == 1 && c->id == 2, "NodePool ids are dense across all node kinds");
    testCheck(pool.get(0) == a && pool.get(1) == b && pool.get(2) == c, "NodePool returns nodes by stable id");
    testCheck(pool.get(kNull) == nullptr && pool.get(10) == nullptr, "NodePool rejects invalid ids");
    pool.markDead(1);
    testCheck(!pool.isAlive(1) && pool.aliveCount() == 2 && pool.size() == 3, "dead nodes retain their id slot");
    pool.markAlive(1);
    testCheck(pool.isAlive(1) && pool.aliveCount() == 3, "removed nodes can be revived for undo");
    pool.clear();
    testCheck(pool.size() == 0 && pool.aliveCount() == 0 && pool.get(0) == nullptr, "clear releases all owned nodes and resets ids");
    testCheck(pool.createTree(4)->id == 0, "new nodes start at id zero after clear");

    TreeNode parent(100, 10);
    TreeNode child(101, 20);
    TreeNode grandchild(102, 30);
    testCheck(TreeNodeLinks::attach(&parent, &child, Side::Left), "tree link helper attaches a child and sets its parent");
    testCheck(TreeNodeLinks::attach(&child, &grandchild, Side::Right), "tree link helper supports nested links");
    testCheck(!TreeNodeLinks::attach(&grandchild, &parent, Side::Left), "tree link helper rejects cycles");
    testCheck(TreeNodeLinks::detach(&parent, Side::Left) == &child && child.parent == nullptr,
              "tree link helper detaches both ends of a parent link");

    EventLog log;
    Event visit(EventType::VISIT);
    visit.alarmCost = 1;
    emit(nullptr, visit);
    emit(&log, visit);
    testCheck(log.size() == 1 && log.totalAlarmCost() == 1, "EventLog emit is null-safe and totals alarm cost");
    testCheck(EventLog::describe(visit).find("Visit") != std::string::npos, "EventLog describes events for the GUI log");
}
