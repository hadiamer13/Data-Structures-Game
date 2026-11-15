// game/SandboxSession.h
// Sandbox backend: real model classes, real events, real snapshots. No SFML.
// Undo = replay of the action history from the seeded start (works for disturbed heaps too).
#ifndef VH_SANDBOXSESSION_H
#define VH_SANDBOXSESSION_H

#include <string>
#include "../contracts/Types.h"
#include "../contracts/Event.h"
#include "../contracts/Containers.h"
#include "../contracts/ViewSnapshot.h"
#include "../model/BinaryTree.h"
#include "../model/BST.h"
#include "../model/AVLTree.h"
#include "../model/MaxHeap.h"

namespace SandboxOp
{
    inline const int Insert = 0;
    inline const int Search = 1;
    inline const int Delete = 2;
    inline const int Range = 3;
    inline const int RotLL = 4;
    inline const int RotRR = 5;
    inline const int RotLR = 6;
    inline const int RotRL = 7;
    inline const int Kth = 8;
    inline const int Rank = 9;
    inline const int CountRange = 10;
    inline const int Min = 11;
    inline const int Max = 12;
    inline const int Succ = 13;
    inline const int Pred = 14;
    inline const int Preorder = 15;
    inline const int Inorder = 16;
    inline const int Postorder = 17;
    inline const int LevelOrder = 18;
    inline const int Mirror = 19;
    inline const int Info = 20;
    inline const int Build = 21;
    inline const int Heapify = 22;
    inline const int SiftUp = 23;
    inline const int Swap = 24;
    inline const int Extract = 25;
    inline const int Sort = 26;
}

struct SandboxResult
{
    bool accepted;          // the operation did what was asked
    bool changed;           // the structure was modified
    std::string message;
    EventArray events;
    int focusId;            // node to ring afterwards (kNull = none)
    ViewSnapshot before;    // animation start (heap insert: includes the new cell)
    ViewSnapshot after;
    bool valid;             // Validator verdict after the operation
    std::string validText;

    SandboxResult() : accepted(false), changed(false), focusId(kNull), valid(true)
    {
    }
};

class SandboxSession
{
public:
    SandboxSession();

    void start(int kind, int seed, bool empty);
    SandboxResult apply(int op, int a, int b);
    bool undo();
    void reset();

    ViewSnapshot snapshot() const;
    bool check(std::string& text) const;    // Validator verdict + text
    int kind() const;
    int steps() const;                      // visits + compares so far
    bool canUndo() const;

    static bool isMutating(int op);

private:
    SandboxSession(const SandboxSession&);
    SandboxSession& operator=(const SandboxSession&);

    void build();
    void replayHistory();
    bool run(int op, int a, int b, EventLog* log, std::string& msg, int& focus);
    bool runBinary(int op, int a, int b, EventLog* log, std::string& msg, int& focus);
    bool runBST(int op, int a, int b, EventLog* log, std::string& msg, int& focus);
    bool runAVL(int op, int a, int b, EventLog* log, std::string& msg, int& focus);
    bool runHeap(int op, int a, int b, EventLog* log, std::string& msg, int& focus);

    int m_kind;
    int m_seed;
    bool m_empty;
    int m_steps;

    BinaryTree m_bin;
    BST m_bst;
    AVLTree m_avl;
    MaxHeap m_heap;

    IntArray m_history;     // stride 4: op, a, b, cost (mutating actions only)
};

#endif