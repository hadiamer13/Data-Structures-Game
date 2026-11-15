#pragma once
// gui/FakeSnapshots.h
// TEMPORARY. Delete this file at integration.
// INTEGRATION (later): game/SnapshotBuilder builds real ViewSnapshots from the model.
//
// Hand-built trees and heaps for testing LayoutEngine, Renderer and the
// level screens before GameSession exists.
// Trees are written as preorder with null markers (like the save format).
// Ids are assigned in preorder (dense, 0..n-1). Parent, height (leaf = 1),
// size and balance (height(left) - height(right)) are COMPUTED from the
// structure, so they can never contradict the tree.
#pragma once

#include "contracts/Types.h"
#include "contracts/Containers.h"
#include "contracts/ViewSnapshot.h"

namespace FakeSnapshots
{
    // Null marker inside the preorder token lists.
    inline const int X = -1000000;

    // Reads tokens[cursor...] and builds one subtree. Returns its id or kNull.
    inline int buildSub(ViewSnapshot& snap, const int* tokens, int count, int& cursor, int parent)
    {
        if (cursor >= count)
        {
            return kNull;
        }
        int token = tokens[cursor];
        cursor++;
        if (token == X)
        {
            return kNull;
        }

        ViewNode node;
        node.id = snap.nodes.size();
        node.key = token;
        node.parent = parent;
        node.alive = true;
        snap.nodes.push(node);
        int id = node.id;

        int left = buildSub(snap, tokens, count, cursor, id);
        int right = buildSub(snap, tokens, count, cursor, id);

        int hl = 0;
        int hr = 0;
        int sl = 0;
        int sr = 0;
        if (left != kNull)
        {
            hl = snap.nodes[left].height;
            sl = snap.nodes[left].size;
        }
        if (right != kNull)
        {
            hr = snap.nodes[right].height;
            sr = snap.nodes[right].size;
        }

        ViewNode& n = snap.nodes[id];
        n.left = left;
        n.right = right;
        n.height = 1 + ((hl > hr) ? hl : hr);
        n.size = 1 + sl + sr;
        n.balance = hl - hr;
        return id;
    }

    inline ViewSnapshot makeTree(int kind, const int* tokens, int count)
    {
        ViewSnapshot snap;
        snap.kind = kind;
        int cursor = 0;
        snap.rootId = buildSub(snap, tokens, count, cursor, kNull);
        return snap;
    }

    inline ViewSnapshot makeHeap(const int* values, int count, int activeSize)
    {
        ViewSnapshot snap;
        snap.kind = TreeKind::MaxHeap;
        snap.rootId = kNull;
        for (int i = 0; i < count; i++)
        {
            snap.heap.array.push(values[i]);
        }
        if (activeSize < 0)
        {
            activeSize = 0;
        }
        if (activeSize > count)
        {
            activeSize = count;
        }
        snap.heap.activeSize = activeSize;
        return snap;
    }

    // ---------------------------------------------------------------- BST

    // Perfect 7-node BST:        50
    //                         30    70
    //                        20 40 60 80
    inline ViewSnapshot bst7()
    {
        const int t[] = { 50, 30, 20, X, X, 40, X, X, 70, 60, X, X, 80, X, X };
        return makeTree(TreeKind::BST, t, 15);
    }

    // Right-leaning chain 1..8 (Act 2 boss: sorted inserts, height n).
    inline ViewSnapshot bstChain8()
    {
        int t[17];
        int n = 0;
        for (int k = 1; k <= 8; k++)
        {
            t[n++] = k;
            t[n++] = X;
        }
        t[n++] = X;
        return makeTree(TreeKind::BST, t, n);
    }

    // ---------------------------------------------------------------- AVL

    // Mixed badges: 10 green (bf 0), 20 yellow (bf +1), 30 RED (bf +2),
    // 40 yellow (bf +1), 50 / 60 / 70 green.
    inline ViewSnapshot avlMixedBadges()
    {
        const int t[] = { 40, 30, 20, 10, X, X, X, X, 60, 50, X, X, 70, X, X };
        return makeTree(TreeKind::AVL, t, 15);
    }

    // LL imbalance at the root (bf +2, left child leans left). Use it to test
    // a right rotation at 50 and the tween between before/after layouts.
    // Preorder ids: 50=0, 30=1, 20=2, 10=3, 40=4, 60=5.
    inline ViewSnapshot avlLLImbalance()
    {
        const int t[] = { 50, 30, 20, 10, X, X, X, 40, X, X, 60, X, X };
        return makeTree(TreeKind::AVL, t, 13);
    }

    // The same tree after rotateRight(50). Ids match the "before" snapshot,
    // only links changed, so nodes tween to their new slots.
    // Pivot 30 becomes root, 40 moves to 50's left, 50 becomes 30's right.
    inline ViewSnapshot avlLLAfterRotation()
    {
        ViewSnapshot snap = avlLLImbalance();
        snap.rootId = 1;

        snap.nodes[1].parent = kNull;
        snap.nodes[1].left = 2;
        snap.nodes[1].right = 0;

        snap.nodes[0].parent = 1;
        snap.nodes[0].left = 4;
        snap.nodes[0].right = 5;

        snap.nodes[4].parent = 0;

        // recompute 50 first, then 30 (the other children are unchanged)
        ViewNode& n50 = snap.nodes[0];
        int h40 = snap.nodes[4].height;
        int h60 = snap.nodes[5].height;
        n50.height = 1 + ((h40 > h60) ? h40 : h60);
        n50.size = 1 + snap.nodes[4].size + snap.nodes[5].size;
        n50.balance = h40 - h60;

        ViewNode& n30 = snap.nodes[1];
        int h20 = snap.nodes[2].height;
        n30.height = 1 + ((h20 > n50.height) ? h20 : n50.height);
        n30.size = 1 + snap.nodes[2].size + n50.size;
        n30.balance = h20 - n50.height;
        return snap;
    }

    // ---------------------------------------------------------------- Heap

    // Valid Max Heap, whole array active (4.1 / 4.2 style).
    inline ViewSnapshot heapFull()
    {
        const int a[] = { 90, 70, 80, 40, 50, 60, 75, 10, 20, 30 };
        return makeHeap(a, 10, 10);
    }

    // Mid Heap Sort: active heap = first 6 cells, last 4 are the sorted region.
    inline ViewSnapshot heapMidSort()
    {
        const int a[] = { 60, 50, 40, 30, 20, 10, 70, 75, 80, 90 };
        return makeHeap(a, 10, 6);
    }
}