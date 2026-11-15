#include "TestSupport.h"
#include "../contracts/Containers.h"
#include "../model/NodePool.h"

void runContainerTests()
{
    IntArray ints;
    for (int i = 0; i < 20; i++) ints.push(i);
    IntArray intsCopy = ints;
    intsCopy[0] = 99;
    testCheck(ints.size() == 20 && ints[19] == 19, "IntArray grows and deep-copies");
    testCheck(intsCopy[0] == 99 && ints[0] == 0 && ints.indexOf(12) == 12, "IntArray indexing and search");
    intsCopy.pop();
    intsCopy.resize(22);
    testCheck(intsCopy.size() == 22 && intsCopy[20] == 0, "IntArray pop and resize");

    BoolArray bools;
    bools.resize(2);
    bools[1] = true;
    BoolArray boolCopy = bools;
    boolCopy[0] = true;
    testCheck(!bools[0] && bools[1] && boolCopy[0], "BoolArray resizes and deep-copies");

    NodePool pool;
    TreeNode* first = pool.createTree(10);
    TreeNode* second = pool.createTree(20);
    BSTNode* third = pool.createBST(30);
    testCheck(first->id == 0 && second->id == 1 && third->id == 2, "NodePool assigns unique dense ids across node kinds");
    testCheck(pool.get(2) == third && pool.aliveCount() == 3, "NodePool lookup and alive count");
    pool.markDead(1);
    testCheck(!pool.isAlive(1) && pool.aliveCount() == 2 && pool.size() == 3, "NodePool preserves dead id slots");
    pool.clear();
    testCheck(pool.size() == 0 && pool.aliveCount() == 0, "NodePool clear deletes and releases every owned node");

    NodePtrArray nodes;
    nodes.push(first);
    nodes.push(second);
    NodePtrArray nodeCopy = nodes;
    testCheck(nodeCopy.size() == 2 && nodeCopy[1] == second, "NodePtrArray copies pointer slots");

    EventArray events;
    Event event(EventType::VISIT);
    events.push(event);
    EventArray eventCopy = events;
    eventCopy[0].id = 8;
    testCheck(events.size() == 1 && events[0].id == kNull && eventCopy[0].id == 8, "EventArray stores independent event values");

    StringArray strings;
    strings.push("alpha");
    strings.push("beta");
    StringArray stringCopy = strings;
    stringCopy[0] = "changed";
    testCheck(strings.size() == 2 && strings[1] == "beta" && stringCopy[0] == "changed", "StringArray grows and deep-copies");

    IntStack intsStack;
    intsStack.push(4);
    intsStack.push(9);
    testCheck(intsStack.size() == 2 && intsStack.top() == 9, "IntStack is last-in first-out");
    intsStack.pop();
    testCheck(intsStack.top() == 4, "IntStack pop exposes the previous item");

    NodeStack nodeStack;
    nodeStack.push(first);
    nodeStack.push(second);
    testCheck(nodeStack.top() == second, "NodeStack stores node pointers");
    nodeStack.pop();
    testCheck(nodeStack.top() == first, "NodeStack pop exposes the previous node");

    IntQueue intsQueue;
    for (int i = 0; i < 12; i++) intsQueue.push(i);
    for (int i = 0; i < 7; i++) intsQueue.pop();
    for (int i = 12; i < 21; i++) intsQueue.push(i);
    testCheck(intsQueue.size() == 14 && intsQueue.front() == 7, "IntQueue preserves FIFO order across wrap and growth");
    IntQueue queueCopy = intsQueue;
    queueCopy.pop();
    testCheck(intsQueue.front() == 7 && queueCopy.front() == 8, "IntQueue deep-copies wrapped contents");

    NodeQueue nodeQueue;
    nodeQueue.push(first);
    nodeQueue.push(second);
    testCheck(nodeQueue.front() == first, "NodeQueue is FIFO");
    nodeQueue.pop();
    testCheck(nodeQueue.front() == second, "NodeQueue advances after pop");
}
