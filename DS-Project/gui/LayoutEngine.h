#pragma once
// Pure math:ViewSnapshot se har node id ka (x, y) aur array cell ki qatar nikalta hai.
// SFML nahi hai. Sirf Point aur ViewSnapshot istemal hote hain.
//
// Coordinates "layout space" mein hain: (0,0) poori drawing (tree + array row) ka top-left hai.
// Zoom aur pan Renderer apply karta hai.
//
// Trees (Binary/BST/AVL): compact tidy-tree, parent apne bachon ke beech center mein,
//   subtrees kabhi overlap nahi karte (contour based), y = depth * vGap.
// maxheap:complete binary tree ke slots. Node id == array index. Slot positions poore array
//   ki length par depend karti hain, is liye Heap Sort ke dauran nodes jump nahi karte.
//   Sirf indices < activeSize ko tree position milti hai (nikale hue values sirf array row mein hote hain).
// Array row: array ke har element ke liye aik cell, tree ke neeche center mein.

#include "../contracts/Types.h"
#include "../contracts/ViewSnapshot.h"

class LayoutEngine
{
  public:
    LayoutEngine();
    ~LayoutEngine();

    void setSpacing(float hGap, float vGap);
    void setArrayStyle(float cellW, float cellH, float cellGap, float gapBelowTree);

    // Sab kuch dobara compute karta hai. Sirf snapshot badalne par call karo.
    void compute(const ViewSnapshot& snap);

    // Doosre engine se positions copy karta hai (tween ke liye "from" layout rakhne ke liye).
    void copyFrom(const LayoutEngine& other);

    // Tree node ko id se dhoondna. Agar !hasNode ho to nodePos (0,0) deta hai.
    bool  hasNode(int id) const;
    Point nodePos(int id) const;
    int nodeCapacity() const;

    // Array row sirf heap. cellPos cell ka CENTRE deta hai.
    int   cellCount() const;
    Point cellPos(int index) const;
    float cellWidth() const;
    float cellHeight() const;

    // Nodes aur cells ka bounding box node centres/cell edges, min (0,0) hota hai.
    float width() const;
    float height() const;

    // from -> to tween ke liye linear interpolation.
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
    int layoutSub(const ViewSnapshot& snap, int id, float* lo, float* hi, int cap);
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
    int m_cap;

    Point* m_cell;
    int m_cellCount;

    int m_guard;
    float m_width;
    float m_height;
};
