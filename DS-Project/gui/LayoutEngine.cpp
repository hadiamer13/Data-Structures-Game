#include "LayoutEngine.h"

namespace
{
    inline float lmin(float a, float b)
    {
        return (a < b) ? a : b;
    }

    inline float lmax(float a, float b)
    {
        return (a > b) ? a : b;
    }

    inline int imin(int a, int b)
    {
        return (a < b) ? a : b;
    }

    inline int imax(int a, int b)
    {
        return (a > b) ? a : b;
    }
}

LayoutEngine::LayoutEngine()
    : m_hGap(60.f), m_vGap(80.f),
      m_cellW(48.f), m_cellH(48.f), m_cellGap(6.f), m_arrayGap(90.f),
      m_pos(0), m_has(0), m_rel(0), m_cap(0),
      m_cell(0), m_cellCount(0),
      m_guard(0), m_width(0.f), m_height(0.f)
{
}

LayoutEngine::~LayoutEngine()
{
    clear();
}

void LayoutEngine::setSpacing(float hGap, float vGap)
{
    m_hGap = hGap;
    m_vGap = vGap;
}

void LayoutEngine::setArrayStyle(float cellW, float cellH, float cellGap, float gapBelowTree)
{
    m_cellW = cellW;
    m_cellH = cellH;
    m_cellGap = cellGap;
    m_arrayGap = gapBelowTree;
}

void LayoutEngine::clear()
{
    delete[] m_pos;
    delete[] m_has;
    delete[] m_rel;
    delete[] m_cell;
    m_pos = 0;
    m_has = 0;
    m_rel = 0;
    m_cell = 0;
    m_cap = 0;
    m_cellCount = 0;
    m_width = 0.f;
    m_height = 0.f;
}

void LayoutEngine::allocNodes(int cap)
{
    if (cap < 0)
    {
        cap = 0;
    }
    m_cap = cap;
    m_pos = new Point[cap + 1];
    m_has = new bool[cap + 1];
    m_rel = new float[cap + 1];
    for (int i = 0; i <= cap; i++)
    {
        m_pos[i].x = 0.f;
        m_pos[i].y = 0.f;
        m_has[i] = false;
        m_rel[i] = 0.f;
    }
}

bool LayoutEngine::validId(const ViewSnapshot& snap, int id) const
{
    if (id < 0 || id >= snap.nodes.size() || id >= m_cap)
    {
        return false;
    }
    return snap.nodes[id].alive;
}

// ---------------------------------------------------------------- compute

void LayoutEngine::compute(const ViewSnapshot& snap)
{
    clear();

    if (snap.kind == TreeKind::MaxHeap)
    {
        layoutHeap(snap);
    }
    else
    {
        layoutTree(snap);
    }
    finish();
}

// ------------------------------------------------------------- tidy tree
//
// layoutSub fills lo[d]/hi[d] = leftmost/rightmost x at depth d of the subtree
// (relative to the subtree root at x = 0) and stores each child's offset to
// its parent in m_rel. Two child subtrees are pushed apart just enough that
// their contours keep m_hGap distance on every shared level.

int LayoutEngine::layoutSub(const ViewSnapshot& snap, int id, float* lo, float* hi, int cap)
{
    if (!validId(snap, id) || cap <= 0)
    {
        return 0;
    }

    m_guard++;
    if (m_guard > m_cap + 8)
    {
        return 0; // cycle protection
    }

    lo[0] = 0.f;
    hi[0] = 0.f;

    if (cap == 1)
    {
        return 1;
    }

    int left = snap.nodes[id].left;
    int right = snap.nodes[id].right;

    float* lL = new float[cap];
    float* hL = new float[cap];
    float* lR = new float[cap];
    float* hR = new float[cap];

    int hl = layoutSub(snap, left, lL, hL, cap - 1);
    int hr = layoutSub(snap, right, lR, hR, cap - 1);

    int h = 1;

    if (hl > 0 || hr > 0)
    {
        float offL = 0.f;
        float offR = 0.f;

        if (hl > 0 && hr > 0)
        {
            float d = m_hGap;
            int common = imin(hl, hr);
            for (int i = 0; i < common; i++)
            {
                d = lmax(d, hL[i] - lR[i] + m_hGap);
            }
            offL = -d * 0.5f;
            offR = d * 0.5f;
        }
        else if (hl > 0)
        {
            offL = -m_hGap * 0.5f;
        }
        else
        {
            offR = m_hGap * 0.5f;
        }

        if (hl > 0)
        {
            m_rel[left] = offL;
        }
        if (hr > 0)
        {
            m_rel[right] = offR;
        }

        h = 1 + imax(hl, hr);
        for (int i = 0; i < h - 1; i++)
        {
            bool hasL = (i < hl);
            bool hasR = (i < hr);
            if (hasL && hasR)
            {
                lo[i + 1] = lmin(offL + lL[i], offR + lR[i]);
                hi[i + 1] = lmax(offL + hL[i], offR + hR[i]);
            }
            else if (hasL)
            {
                lo[i + 1] = offL + lL[i];
                hi[i + 1] = offL + hL[i];
            }
            else
            {
                lo[i + 1] = offR + lR[i];
                hi[i + 1] = offR + hR[i];
            }
        }
    }

    delete[] lL;
    delete[] hL;
    delete[] lR;
    delete[] hR;
    return h;
}

void LayoutEngine::assignSub(const ViewSnapshot& snap, int id, float x, int depth)
{
    if (!validId(snap, id) || depth > m_cap || m_has[id])
    {
        return;
    }
    m_has[id] = true;
    m_pos[id].x = x;
    m_pos[id].y = (float)depth * m_vGap;

    int left = snap.nodes[id].left;
    int right = snap.nodes[id].right;
    if (validId(snap, left))
    {
        assignSub(snap, left, x + m_rel[left], depth + 1);
    }
    if (validId(snap, right))
    {
        assignSub(snap, right, x + m_rel[right], depth + 1);
    }
}

void LayoutEngine::layoutTree(const ViewSnapshot& snap)
{
    allocNodes(snap.nodes.size());

    int alive = 0;
    for (int i = 0; i < snap.nodes.size(); i++)
    {
        if (snap.nodes[i].alive)
        {
            alive++;
        }
    }

    if (!validId(snap, snap.rootId))
    {
        return; // empty tree
    }

    int cap = alive + 1;
    float* lo = new float[cap];
    float* hi = new float[cap];
    m_guard = 0;
    layoutSub(snap, snap.rootId, lo, hi, cap);
    delete[] lo;
    delete[] hi;

    assignSub(snap, snap.rootId, 0.f, 0);
}

// ------------------------------------------------------------------ heap

void LayoutEngine::layoutHeap(const ViewSnapshot& snap)
{
    int total = snap.heap.array.size();
    int active = snap.heap.activeSize;
    if (active > total)
    {
        active = total;
    }
    if (active < 0)
    {
        active = 0;
    }

    allocNodes(total);

    // levels of the FULL array so positions stay stable while the heap shrinks
    int levels = 0;
    int count = 0;
    int levelSize = 1;
    while (count < total)
    {
        count += levelSize;
        levelSize *= 2;
        levels++;
    }

    float leafSlots = 1.f;
    for (int i = 1; i < levels; i++)
    {
        leafSlots *= 2.f;
    }
    float fullWidth = leafSlots * m_hGap;

    int level = 0;
    int levelStart = 0;
    int slots = 1;
    for (int i = 0; i < active; i++)
    {
        while (i >= levelStart + slots)
        {
            levelStart += slots;
            slots *= 2;
            level++;
        }
        int p = i - levelStart;
        float slotW = fullWidth / (float)slots;
        m_pos[i].x = ((float)p + 0.5f) * slotW;
        m_pos[i].y = (float)level * m_vGap;
        m_has[i] = true;
    }

    // array row, centred under the full-width tree
    if (total > 0)
    {
        float treeBottom = (levels > 0) ? (float)(levels - 1) * m_vGap : 0.f;
        layoutArrayRow(total, fullWidth * 0.5f, treeBottom + m_arrayGap);
    }
}

void LayoutEngine::layoutArrayRow(int count, float centreX, float topY)
{
    m_cell = new Point[count];
    m_cellCount = count;
    float totalW = (float)count * (m_cellW + m_cellGap) - m_cellGap;
    float startX = centreX - totalW * 0.5f;
    for (int i = 0; i < count; i++)
    {
        m_cell[i].x = startX + (float)i * (m_cellW + m_cellGap) + m_cellW * 0.5f;
        m_cell[i].y = topY + m_cellH * 0.5f;
    }
}

// ---------------------------------------------------- normalise to (0,0)

void LayoutEngine::finish()
{
    bool any = false;
    float minX = 0.f;
    float minY = 0.f;
    float maxX = 0.f;
    float maxY = 0.f;

    for (int i = 0; i < m_cap; i++)
    {
        if (!m_has[i])
        {
            continue;
        }
        if (!any)
        {
            minX = maxX = m_pos[i].x;
            minY = maxY = m_pos[i].y;
            any = true;
        }
        else
        {
            minX = lmin(minX, m_pos[i].x);
            maxX = lmax(maxX, m_pos[i].x);
            minY = lmin(minY, m_pos[i].y);
            maxY = lmax(maxY, m_pos[i].y);
        }
    }

    for (int i = 0; i < m_cellCount; i++)
    {
        float cl = m_cell[i].x - m_cellW * 0.5f;
        float cr = m_cell[i].x + m_cellW * 0.5f;
        float ct = m_cell[i].y - m_cellH * 0.5f;
        float cb = m_cell[i].y + m_cellH * 0.5f;
        if (!any)
        {
            minX = cl;
            maxX = cr;
            minY = ct;
            maxY = cb;
            any = true;
        }
        else
        {
            minX = lmin(minX, cl);
            maxX = lmax(maxX, cr);
            minY = lmin(minY, ct);
            maxY = lmax(maxY, cb);
        }
    }

    if (!any)
    {
        return;
    }

    for (int i = 0; i < m_cap; i++)
    {
        if (m_has[i])
        {
            m_pos[i].x -= minX;
            m_pos[i].y -= minY;
        }
    }
    for (int i = 0; i < m_cellCount; i++)
    {
        m_cell[i].x -= minX;
        m_cell[i].y -= minY;
    }
    m_width = maxX - minX;
    m_height = maxY - minY;
}

// --------------------------------------------------------------- queries

void LayoutEngine::copyFrom(const LayoutEngine& o)
{
    if (this == &o)
    {
        return;
    }
    clear();
    m_hGap = o.m_hGap;
    m_vGap = o.m_vGap;
    m_cellW = o.m_cellW;
    m_cellH = o.m_cellH;
    m_cellGap = o.m_cellGap;
    m_arrayGap = o.m_arrayGap;
    m_width = o.m_width;
    m_height = o.m_height;

    allocNodes(o.m_cap);
    for (int i = 0; i < o.m_cap; i++)
    {
        m_pos[i] = o.m_pos[i];
        m_has[i] = o.m_has[i];
    }
    if (o.m_cellCount > 0)
    {
        m_cell = new Point[o.m_cellCount];
        m_cellCount = o.m_cellCount;
        for (int i = 0; i < m_cellCount; i++)
        {
            m_cell[i] = o.m_cell[i];
        }
    }
}

bool LayoutEngine::hasNode(int id) const
{
    return id >= 0 && id < m_cap && m_has[id];
}

Point LayoutEngine::nodePos(int id) const
{
    Point p;
    p.x = 0.f;
    p.y = 0.f;
    if (hasNode(id))
    {
        p = m_pos[id];
    }
    return p;
}

int LayoutEngine::nodeCapacity() const
{
    return m_cap;
}

int LayoutEngine::cellCount() const
{
    return m_cellCount;
}

Point LayoutEngine::cellPos(int index) const
{
    Point p;
    p.x = 0.f;
    p.y = 0.f;
    if (index >= 0 && index < m_cellCount)
    {
        p = m_cell[index];
    }
    return p;
}

float LayoutEngine::cellWidth() const
{
    return m_cellW;
}

float LayoutEngine::cellHeight() const
{
    return m_cellH;
}

float LayoutEngine::width() const
{
    return m_width;
}

float LayoutEngine::height() const
{
    return m_height;
}

Point LayoutEngine::mix(const Point& a, const Point& b, float t)
{
    Point p;
    p.x = a.x + (b.x - a.x) * t;
    p.y = a.y + (b.y - a.y) * t;
    return p;
}