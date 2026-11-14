#include "Renderer.h"

namespace
{
    inline float fMin(float a, float b)
    {
        return (a < b) ? a : b;
    }

    inline float fMax(float a, float b)
    {
        return (a > b) ? a : b;
    }

    inline float fClamp(float v, float lo, float hi)
    {
        return fMax(lo, fMin(hi, v));
    }

    inline float fLerp(float a, float b, float t)
    {
        return a + (b - a) * t;
    }

    inline float smooth(float t)
    {
        return t * t * (3.f - 2.f * t);
    }

    inline float fAbs(float v)
    {
        return (v < 0.f) ? -v : v;
    }

    inline sf::Color withAlpha(sf::Color c, float a)
    {
        c.a = (sf::Uint8)(255.f * fClamp(a, 0.f, 1.f));
        return c;
    }

    const float NodeR = 22.f;
    const float TweenSeconds = 0.35f;
    const float MinZoom = 0.02f;
    const float MaxZoom = 4.f;
    const float FitMargin = 60.f;
    const float LabelMinZoom = 0.3f;
    const float BadgeMinZoom = 0.5f;

    // Colour code (never the only signal: ring thickness, double ring, slash, text badges)
    const sf::Color ColNeutral(55, 45, 60);
    const sf::Color ColGreen(40, 120, 65);
    const sf::Color ColYellow(165, 140, 30);
    const sf::Color ColRed(160, 40, 40);
    const sf::Color ColRing(200, 190, 190);
    const sf::Color ColBlue(80, 150, 255);
    const sf::Color ColTrail(110, 150, 230);
    const sf::Color ColGrey(120, 120, 120);
    const sf::Color ColGold(240, 190, 60);
    const sf::Color ColAlert(255, 70, 70);
    const sf::Color ColEdge(170, 160, 160);
    const sf::Color ColCellActive(50, 40, 60);
    const sf::Color ColCellSorted(35, 95, 80);
    const sf::Color ColSorted(90, 200, 160);
    const sf::Color ColText(240, 235, 235);
}

// ----------------------------------------------------------- construction

Renderer::Renderer(const sf::Font& font)
    : m_font(&font),
    m_rx(0.f), m_ry(0.f), m_rw(800.f), m_rh(600.f),
    m_nodes(0), m_cap(0), m_count(0), m_kind(TreeKind::BST), m_activeSize(0),
    m_key(0), m_badge(0), m_cell(0), m_idx(0),
    m_regionA(0), m_regionB(0),
    m_edges(sf::Lines), m_edgesDirty(true),
    m_tween(1.f),
    m_zoom(1.f), m_cx(0.f), m_cy(0.f), m_tZoom(1.f), m_tCx(0.f), m_tCy(0.f),
    m_userMoved(false), m_hasFit(false), m_dragging(false), m_lastMx(0), m_lastMy(0),
    m_swapA(kNull), m_swapB(kNull), m_swapT(0.f)
{
    m_circle.setPointCount(32);
    m_ring.setPointCount(32);
    m_ring.setFillColor(sf::Color::Transparent);
    m_rect.setFillColor(sf::Color::Transparent);

    m_swapTxt[0] = makeText(18);
    m_swapTxt[1] = makeText(18);
    m_regionA = makeText(13);
    m_regionB = makeText(13);
}

Renderer::~Renderer()
{
    for (int i = 0; i < m_cap; i++)
    {
        delete m_key[i];
        delete m_badge[i];
        delete m_cell[i];
        delete m_idx[i];
    }
    delete[] m_key;
    delete[] m_badge;
    delete[] m_cell;
    delete[] m_idx;
    delete[] m_nodes;
    delete m_swapTxt[0];
    delete m_swapTxt[1];
    delete m_regionA;
    delete m_regionB;
}

sf::Text* Renderer::makeText(unsigned int size) const
{
    sf::Text* t = new sf::Text();
    t->setFont(*m_font);
    t->setCharacterSize(size);
    t->setFillColor(ColText);
    return t;
}

static void centreText(sf::Text& t)
{
    sf::FloatRect b = t.getLocalBounds();
    t.setOrigin(b.left + b.width * 0.5f, b.top + b.height * 0.5f);
}

void Renderer::resetNode(RenderNode& r) const
{
    r.present = false;
    r.visible = false;
    r.key = 0;
    r.left = kNull;
    r.right = kNull;
    r.parent = kNull;
    r.height = 0;
    r.size = 0;
    r.balance = 0;
    r.x = r.y = 0.f;
    r.fromX = r.fromY = 0.f;
    r.toX = r.toY = 0.f;
    r.alpha = 0.f;
    r.targetAlpha = 0.f;
    r.pulse = 0.f;
    r.mark = NodeMark::None;
    r.selected = false;
    r.target = false;
}

void Renderer::ensureCapacity(int n)
{
    if (n <= m_cap)
    {
        return;
    }
    int newCap = (m_cap == 0) ? 32 : m_cap;
    while (newCap < n)
    {
        newCap *= 2;
    }

    RenderNode* nn = new RenderNode[newCap];
    sf::Text** nk = new sf::Text*[newCap];
    sf::Text** nb = new sf::Text*[newCap];
    sf::Text** nc = new sf::Text*[newCap];
    sf::Text** ni = new sf::Text*[newCap];

    for (int i = 0; i < m_cap; i++)
    {
        nn[i] = m_nodes[i];
        nk[i] = m_key[i];
        nb[i] = m_badge[i];
        nc[i] = m_cell[i];
        ni[i] = m_idx[i];
    }
    for (int i = m_cap; i < newCap; i++)
    {
        resetNode(nn[i]);
        nk[i] = makeText(18);
        nb[i] = makeText(12);
        nc[i] = makeText(18);
        ni[i] = makeText(12);
        ni[i]->setString(std::to_string(i));
        ni[i]->setFillColor(sf::Color(160, 150, 150));
        centreText(*ni[i]);
    }

    delete[] m_nodes;
    delete[] m_key;
    delete[] m_badge;
    delete[] m_cell;
    delete[] m_idx;
    m_nodes = nn;
    m_key = nk;
    m_badge = nb;
    m_cell = nc;
    m_idx = ni;
    m_cap = newCap;
}

bool Renderer::validIdx(int i) const
{
    return i >= 0 && i < m_count && m_nodes[i].present;
}

// ---------------------------------------------------------------- labels

void Renderer::refreshLabel(int i)
{
    if (i < 0 || i >= m_count)
    {
        return;
    }
    RenderNode& r = m_nodes[i];
    if (!r.present)
    {
        return;
    }

    std::string k = std::to_string(r.key);
    m_key[i]->setString(k);
    centreText(*m_key[i]);
    m_cell[i]->setString(k);
    centreText(*m_cell[i]);

    if (m_kind == TreeKind::AVL)
    {
        std::string s;
        if (r.balance > 1 || r.balance < -1)
        {
            s = "! ";
        }
        s += "h" + std::to_string(r.height) + " b";
        if (r.balance > 0)
        {
            s += "+";
        }
        s += std::to_string(r.balance);
        m_badge[i]->setString(s);
    }
    else if (m_kind == TreeKind::MaxHeap && i == 0)
    {
        m_badge[i]->setString("MAX");
    }
    else
    {
        m_badge[i]->setString("");
    }
    centreText(*m_badge[i]);
}

void Renderer::updateRegionLabels()
{
    m_regionA->setString("ACTIVE HEAP (size " + std::to_string(m_activeSize) + ")");
    m_regionB->setString("SORTED (" + std::to_string(m_count - m_activeSize) + ")");
    m_regionB->setFillColor(ColSorted);
}

// -------------------------------------------------------------- snapshots

void Renderer::setSnapshot(const ViewSnapshot& s, bool animate)
{
    m_kind = s.kind;
    m_layout.compute(s);
    bool heap = (m_kind == TreeKind::MaxHeap);

    int n = heap ? s.heap.array.size() : s.nodes.size();
    ensureCapacity(n);
    int oldCount = m_count;
    m_count = n;

    for (int i = n; i < oldCount; i++)
    {
        m_nodes[i].present = false;
        m_nodes[i].visible = false;
        m_nodes[i].alpha = 0.f;
        m_nodes[i].targetAlpha = 0.f;
    }

    m_activeSize = heap ? s.heap.activeSize : n;
    if (m_activeSize > n)
    {
        m_activeSize = n;
    }

    for (int i = 0; i < n; i++)
    {
        RenderNode& r = m_nodes[i];
        bool was = (i < oldCount) && r.present;
        if (i >= oldCount)
        {
            resetNode(r);
        }
        float curX = r.x;
        float curY = r.y;

        if (heap)
        {
            r.present = true;
            r.key = s.heap.array[i];
            r.left = (2 * i + 1 < n) ? 2 * i + 1 : kNull;
            r.right = (2 * i + 2 < n) ? 2 * i + 2 : kNull;
            r.parent = (i > 0) ? (i - 1) / 2 : kNull;
            r.height = 0;
            r.size = 0;
            r.balance = 0;
            r.visible = (i < m_activeSize);
        }
        else
        {
            const ViewNode& v = s.nodes[i];
            r.present = v.alive && m_layout.hasNode(i);
            r.key = v.key;
            r.left = v.left;
            r.right = v.right;
            r.parent = v.parent;
            r.height = v.height;
            r.size = v.size;
            r.balance = v.balance;
            r.visible = r.present;
        }

        if (!r.present)
        {
            r.targetAlpha = 0.f;
            if (!animate)
            {
                r.alpha = 0.f;
            }
            continue;
        }

        r.targetAlpha = r.visible ? 1.f : 0.f;

        if (m_layout.hasNode(i))
        {
            Point p = m_layout.nodePos(i);
            r.toX = p.x;
            r.toY = p.y;
        }
        else
        {
            // heap node outside the active heap: keep its last position
            r.toX = (was ? curX : 0.f);
            r.toY = (was ? curY : 0.f);
        }

        if (animate && was)
        {
            r.fromX = curX;
            r.fromY = curY;
        }
        else if (animate)
        {
            // new node grows out of its parent's slot
            r.fromX = r.toX;
            r.fromY = r.toY;
            if (r.parent != kNull && m_layout.hasNode(r.parent))
            {
                Point pp = m_layout.nodePos(r.parent);
                r.fromX = pp.x;
                r.fromY = pp.y;
            }
            r.alpha = 0.f;
            r.x = r.fromX;
            r.y = r.fromY;
        }
        else
        {
            r.fromX = r.x = r.toX;
            r.fromY = r.y = r.toY;
            r.alpha = r.targetAlpha;
        }
    }

    for (int i = 0; i < n; i++)
    {
        refreshLabel(i);
    }
    updateRegionLabels();

    m_tween = animate ? 0.f : 1.f;
    m_swapA = kNull;
    m_swapB = kNull;
    m_edgesDirty = true;

    if (!m_hasFit)
    {
        fit(false);
        m_hasFit = true;
    }
    else if (!m_userMoved)
    {
        fit(animate);
    }
}

void Renderer::animateTo(const ViewSnapshot& snap)
{
    setSnapshot(snap, true);
}

void Renderer::snapTo(const ViewSnapshot& snap)
{
    setSnapshot(snap, false);
}

// ----------------------------------------------------------- highlights

void Renderer::clearHighlights()
{
    for (int i = 0; i < m_count; i++)
    {
        m_nodes[i].mark = NodeMark::None;
        m_nodes[i].pulse = 0.f;
    }
    m_edgesDirty = true;
}

void Renderer::setMark(int id, int mark)
{
    if (validIdx(id))
    {
        m_nodes[id].mark = mark;
        m_edgesDirty = true;
    }
}

void Renderer::setSelected(int id)
{
    for (int i = 0; i < m_count; i++)
    {
        m_nodes[i].selected = (i == id);
    }
}

void Renderer::setTarget(int id)
{
    for (int i = 0; i < m_count; i++)
    {
        m_nodes[i].target = (i == id);
    }
}

void Renderer::markSubtree(int id, int mark, int depth)
{
    if (!validIdx(id) || depth > m_count)
    {
        return;
    }
    m_nodes[id].mark = mark;
    markSubtree(m_nodes[id].left, mark, depth + 1);
    markSubtree(m_nodes[id].right, mark, depth + 1);
}

void Renderer::setActiveSize(int n)
{
    if (n < 0)
    {
        n = 0;
    }
    if (n > m_count)
    {
        n = m_count;
    }
    m_activeSize = n;
    for (int i = 0; i < m_count; i++)
    {
        m_nodes[i].visible = (i < n);
        m_nodes[i].targetAlpha = (i < n && m_nodes[i].present) ? 1.f : 0.f;
    }
    updateRegionLabels();
    m_edgesDirty = true;
}

// ------------------------------------------------------------------ swaps

void Renderer::startSwap(int a, int b)
{
    if (!validIdx(a) || !validIdx(b) || a == b)
    {
        return;
    }
    m_swapA = a;
    m_swapB = b;
    m_swapT = 0.f;
    m_swapTxt[0]->setString(std::to_string(m_nodes[a].key));
    m_swapTxt[1]->setString(std::to_string(m_nodes[b].key));
    centreText(*m_swapTxt[0]);
    centreText(*m_swapTxt[1]);
    m_nodes[a].mark = NodeMark::Compare;
    m_nodes[b].mark = NodeMark::Compare;
    m_edgesDirty = true;
}

void Renderer::finishSwap()
{
    if (m_swapA == kNull || m_swapB == kNull)
    {
        return;
    }
    int t = m_nodes[m_swapA].key;
    m_nodes[m_swapA].key = m_nodes[m_swapB].key;
    m_nodes[m_swapB].key = t;
    int a = m_swapA;
    int b = m_swapB;
    m_swapA = kNull;
    m_swapB = kNull;
    refreshLabel(a);
    refreshLabel(b);
    m_nodes[a].pulse = 1.f;
    m_nodes[b].pulse = 1.f;
}

// ----------------------------------------------------------------- events

void Renderer::beginEvent(const Event& e)
{
    int t = e.type;

    if (t == EventType::VISIT)
    {
        for (int i = 0; i < m_count; i++)
        {
            if (m_nodes[i].mark == NodeMark::Visit || m_nodes[i].mark == NodeMark::Compare)
            {
                m_nodes[i].mark = NodeMark::Trail;
            }
        }
        if (validIdx(e.id))
        {
            m_nodes[e.id].mark = NodeMark::Visit;
            m_nodes[e.id].pulse = 1.f;
        }
        m_edgesDirty = true;
    }
    else if (t == EventType::COMPARE)
    {
        if (validIdx(e.id))
        {
            m_nodes[e.id].mark = NodeMark::Compare;
            m_nodes[e.id].pulse = 1.f;
        }
        m_edgesDirty = true;
    }
    else if (t == EventType::INSERT || t == EventType::UNLINK || t == EventType::ROTATE)
    {
        if (validIdx(e.id))
        {
            m_nodes[e.id].pulse = 1.f;
        }
    }
    else if (t == EventType::SWAP_VALUE)
    {
        startSwap(e.id, e.id2);
    }
    else if (t == EventType::UPDATE_HEIGHT || t == EventType::UPDATE_SIZE)
    {
        if (validIdx(e.id))
        {
            if (t == EventType::UPDATE_HEIGHT)
            {
                m_nodes[e.id].height = e.newV;
                m_nodes[e.id].balance = m_nodes[e.id].balance; // bf arrives with the next snapshot
            }
            else
            {
                m_nodes[e.id].size = e.newV;
            }
            m_nodes[e.id].pulse = 1.f;
            refreshLabel(e.id);
        }
    }
    else if (t == EventType::BALANCE_ALERT)
    {
        if (validIdx(e.id))
        {
            m_nodes[e.id].mark = NodeMark::Violation;
            m_nodes[e.id].pulse = 1.f;
            m_edgesDirty = true;
        }
    }
    else if (t == EventType::HIGHLIGHT_SUBTREE)
    {
        // ASSUMPTION: mode is carried in e.result using HighlightMode constants.
        int mark = NodeMark::Visit;
        if (e.result == HighlightMode::Skipped)
        {
            mark = NodeMark::Skipped;
        }
        else if (e.result == HighlightMode::Result)
        {
            mark = NodeMark::Result;
        }
        markSubtree(e.id, mark, 0);
        m_edgesDirty = true;
    }
    else if (t == EventType::HEAP_SWAP)
    {
        startSwap(e.idx, e.idx2);
    }
    else if (t == EventType::HEAPIFY_START)
    {
        if (validIdx(e.idx))
        {
            m_nodes[e.idx].mark = NodeMark::Visit;
            m_nodes[e.idx].pulse = 1.f;
            m_edgesDirty = true;
        }
    }
    else if (t == EventType::EXTRACT_MAX)
    {
        if (validIdx(e.idx))
        {
            m_nodes[e.idx].mark = NodeMark::Result;
            m_nodes[e.idx].pulse = 1.f;
            m_edgesDirty = true;
        }
    }
    else if (t == EventType::MARK_SORTED)
    {
        if (validIdx(e.idx))
        {
            m_nodes[e.idx].mark = NodeMark::Result;
            m_nodes[e.idx].pulse = 1.f;
        }
    }
    else if (t == EventType::HEAP_SORT_COMPLETE)
    {
        for (int i = 0; i < m_count; i++)
        {
            m_nodes[i].mark = NodeMark::Result;
            m_nodes[i].pulse = 1.f;
        }
        m_edgesDirty = true;
    }
}

void Renderer::progress(const Event& e, float t)
{
    if (e.type == EventType::SWAP_VALUE || e.type == EventType::HEAP_SWAP)
    {
        m_swapT = smooth(fClamp(t, 0.f, 1.f));
    }
}

void Renderer::endEvent(const Event& e)
{
    int t = e.type;
    if (t == EventType::SWAP_VALUE || t == EventType::HEAP_SWAP)
    {
        finishSwap();
    }
    else if (t == EventType::HEAP_SIZE_CHANGE)
    {
        setActiveSize(e.newV);
    }
    else if (t == EventType::HEAPIFY_DONE)
    {
        for (int i = 0; i < m_count; i++)
        {
            if (m_nodes[i].mark == NodeMark::Visit || m_nodes[i].mark == NodeMark::Compare
                || m_nodes[i].mark == NodeMark::Trail)
            {
                m_nodes[i].mark = NodeMark::None;
            }
        }
        m_edgesDirty = true;
    }
}

// ----------------------------------------------------------------- update

void Renderer::update(float dt)
{
    if (dt > 0.05f)
    {
        dt = 0.05f;
    }

    if (m_tween < 1.f)
    {
        m_tween = fMin(1.f, m_tween + dt / TweenSeconds);
        float k = smooth(m_tween);
        for (int i = 0; i < m_count; i++)
        {
            RenderNode& r = m_nodes[i];
            r.x = fLerp(r.fromX, r.toX, k);
            r.y = fLerp(r.fromY, r.toY, k);
        }
        m_edgesDirty = true;
    }

    for (int i = 0; i < m_count; i++)
    {
        RenderNode& r = m_nodes[i];
        if (r.alpha != r.targetAlpha)
        {
            float step = dt * 4.f;
            if (r.alpha < r.targetAlpha)
            {
                r.alpha = fMin(r.targetAlpha, r.alpha + step);
            }
            else
            {
                r.alpha = fMax(r.targetAlpha, r.alpha - step);
            }
            m_edgesDirty = true;
        }
        if (r.pulse > 0.f)
        {
            r.pulse = fMax(0.f, r.pulse - dt * 2.5f);
        }
    }

    float k = fMin(1.f, dt * 10.f);
    m_zoom += (m_tZoom - m_zoom) * k;
    m_cx += (m_tCx - m_cx) * k;
    m_cy += (m_tCy - m_cy) * k;
}

// ----------------------------------------------------------------- camera

void Renderer::setCanvasRect(float x, float y, float w, float h)
{
    m_rx = x;
    m_ry = y;
    m_rw = fMax(1.f, w);
    m_rh = fMax(1.f, h);
    if (m_hasFit && !m_userMoved)
    {
        fit(false);
    }
}

void Renderer::fit(bool animate)
{
    float bw = m_layout.width() + 2.f * FitMargin;
    float bh = m_layout.height() + 2.f * FitMargin;
    float z = fMin(m_rw / bw, m_rh / bh);
    z = fClamp(z, MinZoom, 1.2f);
    m_tZoom = z;
    m_tCx = m_layout.width() * 0.5f;
    m_tCy = m_layout.height() * 0.5f;
    if (!animate)
    {
        m_zoom = m_tZoom;
        m_cx = m_tCx;
        m_cy = m_tCy;
    }
    m_userMoved = false;
}

float Renderer::zoom() const
{
    return m_zoom;
}

bool Renderer::inCanvas(float sx, float sy) const
{
    return sx >= m_rx && sx <= m_rx + m_rw && sy >= m_ry && sy <= m_ry + m_rh;
}

void Renderer::screenToWorld(float sx, float sy, float& wx, float& wy) const
{
    wx = m_cx + (sx - (m_rx + m_rw * 0.5f)) / m_zoom;
    wy = m_cy + (sy - (m_ry + m_rh * 0.5f)) / m_zoom;
}

void Renderer::zoomAt(float sx, float sy, float factor)
{
    float wx;
    float wy;
    screenToWorld(sx, sy, wx, wy);
    m_zoom = fClamp(m_zoom * factor, MinZoom, MaxZoom);
    m_cx = wx - (sx - (m_rx + m_rw * 0.5f)) / m_zoom;
    m_cy = wy - (sy - (m_ry + m_rh * 0.5f)) / m_zoom;
    m_tZoom = m_zoom;
    m_tCx = m_cx;
    m_tCy = m_cy;
    m_userMoved = true;
}

void Renderer::panBy(float dx, float dy)
{
    m_cx -= dx / m_zoom;
    m_cy -= dy / m_zoom;
    m_tCx = m_cx;
    m_tCy = m_cy;
    m_tZoom = m_zoom;
    m_userMoved = true;
}

bool Renderer::handleCameraEvent(const sf::Event& ev)
{
    if (ev.type == sf::Event::MouseWheelScrolled)
    {
        float sx = (float)ev.mouseWheelScroll.x;
        float sy = (float)ev.mouseWheelScroll.y;
        if (inCanvas(sx, sy))
        {
            zoomAt(sx, sy, (ev.mouseWheelScroll.delta > 0.f) ? 1.1f : 1.f / 1.1f);
            return true;
        }
    }
    else if (ev.type == sf::Event::MouseButtonPressed)
    {
        float sx = (float)ev.mouseButton.x;
        float sy = (float)ev.mouseButton.y;
        if (inCanvas(sx, sy))
        {
            bool bg = (pick(sx, sy) == kNull) && (pickCell(sx, sy) == kNull);
            if (ev.mouseButton.button == sf::Mouse::Middle
                || (ev.mouseButton.button == sf::Mouse::Left && bg))
            {
                m_dragging = true;
                m_lastMx = ev.mouseButton.x;
                m_lastMy = ev.mouseButton.y;
            }
        }
    }
    else if (ev.type == sf::Event::MouseButtonReleased)
    {
        m_dragging = false;
    }
    else if (ev.type == sf::Event::MouseMoved)
    {
        if (m_dragging)
        {
            panBy((float)(ev.mouseMove.x - m_lastMx), (float)(ev.mouseMove.y - m_lastMy));
            m_lastMx = ev.mouseMove.x;
            m_lastMy = ev.mouseMove.y;
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------- picking

int Renderer::pick(float sx, float sy) const
{
    if (!inCanvas(sx, sy))
    {
        return kNull;
    }
    float wx;
    float wy;
    screenToWorld(sx, sy, wx, wy);
    int best = kNull;
    float bestD = NodeR * NodeR * 1.3f;
    for (int i = 0; i < m_count; i++)
    {
        const RenderNode& r = m_nodes[i];
        if (!r.present || !r.visible || r.alpha < 0.3f)
        {
            continue;
        }
        float dx = wx - r.x;
        float dy = wy - r.y;
        float d = dx * dx + dy * dy;
        if (d <= bestD)
        {
            bestD = d;
            best = i;
        }
    }
    return best;
}

int Renderer::pickCell(float sx, float sy) const
{
    if (!isHeap() || !inCanvas(sx, sy))
    {
        return kNull;
    }
    float wx;
    float wy;
    screenToWorld(sx, sy, wx, wy);
    int n = m_layout.cellCount();
    for (int i = 0; i < n; i++)
    {
        Point c = m_layout.cellPos(i);
        if (fAbs(wx - c.x) <= m_layout.cellWidth() * 0.5f && fAbs(wy - c.y) <= m_layout.cellHeight() * 0.5f)
        {
            return i;
        }
    }
    return kNull;
}

bool Renderer::nodeScreenPos(int id, float& sx, float& sy) const
{
    if (id < 0 || id >= m_count || !m_nodes[id].present)
    {
        return false;
    }
    sx = m_rx + m_rw * 0.5f + (m_nodes[id].x - m_cx) * m_zoom;
    sy = m_ry + m_rh * 0.5f + (m_nodes[id].y - m_cy) * m_zoom;
    return true;
}

bool Renderer::nodeInfo(int id, int& key, int& height, int& size, int& balance, int& depth) const
{
    if (id < 0 || id >= m_count || !m_nodes[id].present)
    {
        return false;
    }
    const RenderNode& r = m_nodes[id];
    key = r.key;
    height = r.height;
    size = r.size;
    balance = r.balance;
    depth = 0;
    int p = r.parent;
    int guard = 0;
    while (p != kNull && p >= 0 && p < m_count && guard <= m_count)
    {
        depth++;
        guard++;
        p = m_nodes[p].parent;
    }
    return true;
}

int Renderer::kind() const
{
    return m_kind;
}

bool Renderer::isHeap() const
{
    return m_kind == TreeKind::MaxHeap;
}

// ---------------------------------------------------------------- drawing

void Renderer::rebuildEdges()
{
    m_edges.clear();
    for (int i = 0; i < m_count; i++)
    {
        const RenderNode& p = m_nodes[i];
        if (!p.present || p.alpha < 0.02f)
        {
            continue;
        }
        for (int k = 0; k < 2; k++)
        {
            int c = (k == 0) ? p.left : p.right;
            if (c < 0 || c >= m_count)
            {
                continue;
            }
            const RenderNode& ch = m_nodes[c];
            if (!ch.present || ch.alpha < 0.02f)
            {
                continue;
            }
            sf::Color col = ColEdge;
            if (ch.mark == NodeMark::Visit || ch.mark == NodeMark::Compare || ch.mark == NodeMark::Trail)
            {
                col = ColBlue;
            }
            else if (ch.mark == NodeMark::Skipped)
            {
                col = ColGrey;
            }
            col = withAlpha(col, fMin(p.alpha, ch.alpha));
            m_edges.append(sf::Vertex(sf::Vector2f(p.x, p.y), col));
            m_edges.append(sf::Vertex(sf::Vector2f(ch.x, ch.y), col));
        }
    }
    m_edgesDirty = false;
}

void Renderer::drawNode(sf::RenderWindow& w, int i)
{
    const RenderNode& r = m_nodes[i];
    float scale = (0.6f + 0.4f * r.alpha) * (1.f + 0.25f * smooth(r.pulse));
    float R = NodeR * scale;
    bool skipped = (r.mark == NodeMark::Skipped);
    float a = r.alpha * (skipped ? 0.45f : 1.f);

    sf::Color fill = ColNeutral;
    if (m_kind == TreeKind::AVL)
    {
        int b = (r.balance < 0) ? -r.balance : r.balance;
        fill = (b == 0) ? ColGreen : ((b == 1) ? ColYellow : ColRed);
    }

    sf::Color outline = ColRing;
    float thick = 2.f;
    if (r.mark == NodeMark::Visit)
    {
        outline = ColBlue;
        thick = 5.f;
    }
    else if (r.mark == NodeMark::Compare)
    {
        outline = ColBlue;
        thick = 4.f;
    }
    else if (r.mark == NodeMark::Trail)
    {
        outline = ColTrail;
        thick = 2.5f;
    }
    else if (skipped)
    {
        outline = ColGrey;
        thick = 1.5f;
    }
    else if (r.mark == NodeMark::Result)
    {
        outline = ColGold;
        thick = 4.f;
    }
    else if (r.mark == NodeMark::Violation)
    {
        outline = ColAlert;
        thick = 5.f;
    }

    m_circle.setRadius(R);
    m_circle.setOrigin(R, R);
    m_circle.setPosition(r.x, r.y);
    m_circle.setFillColor(withAlpha(fill, a));
    m_circle.setOutlineThickness(thick);
    m_circle.setOutlineColor(withAlpha(outline, a));
    w.draw(m_circle);

    if (r.mark == NodeMark::Result || r.target)
    {
        m_ring.setRadius(R + 7.f);
        m_ring.setOrigin(R + 7.f, R + 7.f);
        m_ring.setPosition(r.x, r.y);
        m_ring.setOutlineThickness(2.f);
        m_ring.setOutlineColor(withAlpha(ColGold, a));
        w.draw(m_ring);
    }
    if (r.selected)
    {
        m_ring.setRadius(R + 4.f);
        m_ring.setOrigin(R + 4.f, R + 4.f);
        m_ring.setPosition(r.x, r.y);
        m_ring.setOutlineThickness(1.5f);
        m_ring.setOutlineColor(withAlpha(sf::Color::White, a));
        w.draw(m_ring);
    }
    if (skipped)
    {
        sf::Vertex line[2];
        line[0] = sf::Vertex(sf::Vector2f(r.x - R * 0.7f, r.y + R * 0.7f), withAlpha(ColGrey, a));
        line[1] = sf::Vertex(sf::Vector2f(r.x + R * 0.7f, r.y - R * 0.7f), withAlpha(ColGrey, a));
        w.draw(line, 2, sf::Lines);
    }

    if (m_zoom >= LabelMinZoom)
    {
        bool travelling = (i == m_swapA || i == m_swapB);
        if (!travelling)
        {
            m_key[i]->setFillColor(withAlpha(ColText, a));
            m_key[i]->setPosition(r.x, r.y);
            w.draw(*m_key[i]);
        }
    }

    bool avl = (m_kind == TreeKind::AVL);
    bool maxTag = (isHeap() && i == 0);
    if (m_zoom >= BadgeMinZoom && (avl || maxTag))
    {
        sf::FloatRect b = m_badge[i]->getLocalBounds();
        float bw = b.width + 10.f;
        float bh = b.height + 8.f;
        float bx = avl ? r.x + R * 0.9f : r.x;
        float by = avl ? r.y - R * 1.25f : r.y - R - 14.f;

        sf::Color bg = ColGold;
        sf::Color tx = sf::Color(20, 15, 15);
        if (avl)
        {
            int bf = (r.balance < 0) ? -r.balance : r.balance;
            if (bf == 0)
            {
                bg = ColGreen;
                tx = ColText;
            }
            else if (bf == 1)
            {
                bg = ColYellow;
            }
            else
            {
                bg = ColRed;
                tx = ColText;
            }
        }
        m_rect.setSize(sf::Vector2f(bw, bh));
        m_rect.setOrigin(bw * 0.5f, bh * 0.5f);
        m_rect.setPosition(bx, by);
        m_rect.setFillColor(withAlpha(bg, a));
        m_rect.setOutlineThickness(1.5f);
        m_rect.setOutlineColor(withAlpha(ColRing, a));
        w.draw(m_rect);
        m_rect.setFillColor(sf::Color::Transparent);

        m_badge[i]->setFillColor(withAlpha(tx, a));
        m_badge[i]->setPosition(bx, by);
        w.draw(*m_badge[i]);
    }
}

void Renderer::drawTravelLabel(sf::RenderWindow& w, const Point& a, const Point& b)
{
    // value of A travels to B's slot, value of B travels to A's slot
    m_swapTxt[0]->setPosition(a.x + (b.x - a.x) * m_swapT, a.y + (b.y - a.y) * m_swapT);
    m_swapTxt[1]->setPosition(b.x + (a.x - b.x) * m_swapT, b.y + (a.y - b.y) * m_swapT);
    w.draw(*m_swapTxt[0]);
    w.draw(*m_swapTxt[1]);
}

void Renderer::drawArray(sf::RenderWindow& w, float left, float right)
{
    int n = m_layout.cellCount();
    float cw = m_layout.cellWidth();
    float ch = m_layout.cellHeight();
    bool labels = (m_zoom >= LabelMinZoom);

    for (int i = 0; i < n; i++)
    {
        Point c = m_layout.cellPos(i);
        if (c.x + cw < left || c.x - cw > right)
        {
            continue;
        }
        const RenderNode& r = m_nodes[i];
        bool active = (i < m_activeSize);

        sf::Color outline = active ? ColRing : ColSorted;
        float thick = active ? 2.f : 3.f;
        if (r.mark == NodeMark::Visit || r.mark == NodeMark::Compare)
        {
            outline = ColBlue;
            thick = 4.f;
        }
        else if (r.mark == NodeMark::Result)
        {
            outline = ColGold;
            thick = 4.f;
        }

        float s = 1.f + 0.15f * smooth(r.pulse);
        m_rect.setSize(sf::Vector2f(cw * s, ch * s));
        m_rect.setOrigin(cw * s * 0.5f, ch * s * 0.5f);
        m_rect.setPosition(c.x, c.y);
        m_rect.setFillColor(active ? ColCellActive : ColCellSorted);
        m_rect.setOutlineThickness(thick);
        m_rect.setOutlineColor(outline);
        w.draw(m_rect);
        m_rect.setFillColor(sf::Color::Transparent);

        if (labels)
        {
            if (i != m_swapA && i != m_swapB)
            {
                m_cell[i]->setFillColor(ColText);
                m_cell[i]->setPosition(c.x, c.y);
                w.draw(*m_cell[i]);
            }
            m_idx[i]->setPosition(c.x, c.y + ch * 0.5f + 12.f);
            w.draw(*m_idx[i]);
        }
    }

    if (labels && m_swapA != kNull && m_swapB != kNull)
    {
        drawTravelLabel(w, m_layout.cellPos(m_swapA), m_layout.cellPos(m_swapB));
    }

    // region frames + text labels (colour is not the only signal)
    if (n > 0)
    {
        float top = m_layout.cellPos(0).y - ch * 0.5f - 6.f;
        if (m_activeSize > 0)
        {
            float x0 = m_layout.cellPos(0).x - cw * 0.5f - 3.f;
            float x1 = m_layout.cellPos(m_activeSize - 1).x + cw * 0.5f + 3.f;
            m_rect.setSize(sf::Vector2f(x1 - x0, ch + 12.f));
            m_rect.setOrigin(0.f, 0.f);
            m_rect.setPosition(x0, top);
            m_rect.setOutlineThickness(1.5f);
            m_rect.setOutlineColor(ColRing);
            w.draw(m_rect);
            if (labels)
            {
                m_regionA->setPosition(x0, top - 20.f);
                w.draw(*m_regionA);
            }
        }
        if (m_activeSize < n)
        {
            float x0 = m_layout.cellPos(m_activeSize).x - cw * 0.5f - 3.f;
            float x1 = m_layout.cellPos(n - 1).x + cw * 0.5f + 3.f;
            m_rect.setSize(sf::Vector2f(x1 - x0, ch + 12.f));
            m_rect.setOrigin(0.f, 0.f);
            m_rect.setPosition(x0, top);
            m_rect.setOutlineThickness(3.f);
            m_rect.setOutlineColor(ColSorted);
            w.draw(m_rect);
            if (labels)
            {
                m_regionB->setPosition(x0, top - 20.f);
                w.draw(*m_regionB);
            }
        }
        m_rect.setOutlineThickness(0.f);
    }
}

void Renderer::draw(sf::RenderWindow& window)
{
    if (m_count == 0)
    {
        return;
    }
    if (m_edgesDirty)
    {
        rebuildEdges();
    }

    sf::Vector2u ws = window.getSize();
    sf::View view(sf::Vector2f(m_cx, m_cy), sf::Vector2f(m_rw / m_zoom, m_rh / m_zoom));
    view.setViewport(sf::FloatRect(m_rx / (float)ws.x, m_ry / (float)ws.y,
        m_rw / (float)ws.x, m_rh / (float)ws.y));
    window.setView(view);

    float halfW = m_rw / (2.f * m_zoom) + 60.f;
    float halfH = m_rh / (2.f * m_zoom) + 60.f;
    float left = m_cx - halfW;
    float right = m_cx + halfW;
    float top = m_cy - halfH;
    float bottom = m_cy + halfH;

    window.draw(m_edges);

    for (int i = 0; i < m_count; i++)
    {
        const RenderNode& r = m_nodes[i];
        if (!r.present || r.alpha < 0.02f)
        {
            continue;
        }
        if (r.x < left || r.x > right || r.y < top || r.y > bottom)
        {
            continue;
        }
        drawNode(window, i);
    }

    if (m_zoom >= LabelMinZoom && m_swapA != kNull && m_swapB != kNull)
    {
        Point a;
        a.x = m_nodes[m_swapA].x;
        a.y = m_nodes[m_swapA].y;
        Point b;
        b.x = m_nodes[m_swapB].x;
        b.y = m_nodes[m_swapB].y;
        drawTravelLabel(window, a, b);
    }

    if (isHeap())
    {
        drawArray(window, left, right);
    }

    window.setView(window.getDefaultView());
}
