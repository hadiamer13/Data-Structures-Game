#pragma once
// Pure math: ViewSnapshot -> (x, y) per node id + array cell row.
// No SFML. Uses only Point and ViewSnapshot from contracts/.
//
// Coordinates are "layout space": origin (0,0) = top-left of the whole
// drawing (tree + array row). The Renderer applies camera zoom/pan.
//
// Trees (Binary/BST/AVL): compact tidy-tree, parent centred over its
//   children, subtrees never overlap (contour based), y = depth * vGap.
// MaxHeap: complete-binary-tree slots. Node id == array index. Slot
//   positions depend on the full array length, so nodes do NOT jump when
//   the active heap shrinks during Heap Sort. Only indices < activeSize get
//   a tree position (extracted values live only in the array row).
// Array row: one cell per array element, centred below the tree.

#include "contracts/Types.h"
#include "contracts/ViewSnapshot.h"

class LayoutEngine
{
public:
    LayoutEngine();
    ~LayoutEngine();

    void setSpacing(float hGap, float vGap);
    void setArrayStyle(float cellW, float cellH, float cellGap, float gapBelowTree);

    // Recompute everything. Call only when the snapshot changed.
    void compute(const ViewSnapshot& snap);

    // Copy positions from another engine (keep the "from" layout for tweening).
    void copyFrom(const LayoutEngine& other);

    // Tree node lookup by id. nodePos is (0,0) if !hasNode.
    bool  hasNode(int id) const;
    Point nodePos(int id) const;
    int   nodeCapacity() const;

    // Array row (heap only). cellPos = cell CENTRE.
    int   cellCount() const;
    Point cellPos(int index) const;
    float cellWidth() const;
    float cellHeight() const;

    // Bounding box of nodes + cells (node centres / cell edges), min is (0,0).
    float width() const;
    float height() const;

    // Linear interpolation helper for from->to tweens.
    static Point mix(const Point& a, const Point& b, float t);

private:
    LayoutEngine(const LayoutEngine&);
    LayoutEngine& operator=(const LayoutEngine&);

    void clear();
    void allocNodes(int cap);
    void layoutTree(const ViewSnapshot& snap);
    void layoutHeap(const ViewSnapshot& snap);
    void layoutArrayRow(int count, float centreX, float topY);
    void finish();

    bool validId(const ViewSnapshot& snap, int id) const;
    int  layoutSub(const ViewSnapshot& snap, int id, float* lo, float* hi, int cap);
    void assignSub(const ViewSnapshot& snap, int id, float x, int depth);

    float m_hGap;
    float m_vGap;
    float m_cellW;
    float m_cellH;
    float m_cellGap;
    float m_arrayGap;

    Point* m_pos;
    bool* m_has;
    float* m_rel;
    int    m_cap;

    Point* m_cell;
    int    m_cellCount;

    int   m_guard;
    float m_width;
    float m_height;
};

