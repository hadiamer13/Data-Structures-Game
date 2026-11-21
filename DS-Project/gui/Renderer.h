#pragma once
// Renderer ek ViewSnapshot (tree ya heap + array row) ko canvas rectangle mein draw karta hai.
// Camera (zoom / pan / fit), picking aur har event ki visual state isi ke paas hai.
// IEventVisualizer implement karta hai.
//
// Events aur snapshots kaise kaam karte hain (yeh zaroor samjho):
//  * Events sirf EFFECTS chalate hain: pulses, highlights, arrows, swap ke travelling labels,
//    heap-size changes, sorted marks.
//  * Structure (INSERT / UNLINK / ROTATE ke natijay) controller animateTo(snapshotAfter) call karke
//    lagata hai (nodes naye slots tak tween hote hain, naye nodes parent se nikalte hain,
//    hatne wale nodes fade hote hain). snapTo() = hard sync aur reset.
//  * Ids stable hain, is liye rotation ka matlab hai "wohi ids, naye links, naye slots".

#include <string>
#include <SFML/Graphics.hpp>
#include "IEventVisualizer.h"
#include "LayoutEngine.h"

namespace NodeMark
{
    inline const int None = 0;
    inline const int Visit = 1;     // current visit (motay blue ring)
    inline const int Compare = 2;   // compare ho raha hai (blue ring)
    inline const int Skipped = 3;
    inline const int Result = 4;    // target / result 
    inline const int Violation = 5; // motay laal ring
    inline const int Trail = 6;     // pehle visit hua 
}

namespace NodeFx
{
    inline const int None = 0;
    inline const int Insert = 1;   // green ring
    inline const int Unlink = 2;   // Red ring
    inline const int Rotate = 3;   
}

struct RenderNode
{
    bool present;   // snapshot mein maujood hai (heap: cell maujood hai)
    bool visible;   // tree view mein dikhta hai (heap: index < activeSize)
    int key;
    int left;
    int right;
    int parent;
    int height;
    int size;
    int balance;
    float x, y;
    float fromX, fromY;
    float toX, toY;
    float alpha;
    float targetAlpha;
    float pulse;
    int mark;
    bool selected;
    bool target;
    bool dying;     // hatne wala node jo fade ho raha hai (ab bhi draw hota hai, edge sikud rahi hai)
    float fx;       // effect ki shiddat 0..1
    int fxKind;     // NodeFx::*
};

class Renderer : public IEventVisualizer
{
  public:
    explicit Renderer(const sf::Font& font);
    ~Renderer();

    // Screenpixel rectangle jiske andar tree/heap draw hota hai (usi tak clip hota hai).
    void setCanvasRect(float x, float y, float w, float h);

    // Snapshots
    void setSnapshot(const ViewSnapshot& snap, bool animate);
    void animateTo(const ViewSnapshot& snap) override;
    void snapTo(const ViewSnapshot& snap) override;

    // IEventVisualizer
    void beginEvent(const Event& e) override;
    void progress(const Event& e, float t) override;
    void endEvent(const Event& e) override;

    // Frame
    void update(float dt);
    void draw(sf::RenderWindow& window);

    // Camera
    void fit(bool animate);
    void zoomAt(float sx, float sy, float factor);
    void panBy(float dxScreen, float dyScreen);
    bool handleCameraEvent(const sf::Event& ev);     // wheel zoom, background drag / middle drag pan
    float zoom() const;

    // Picking (screen pixels). Kuch na mile to -1.
    int pick(float sx, float sy) const;              // tree node id or heap: index
    int pickCell(float sx, float sy) const;          // array cell index (sirf heap)
    bool nodeScreenPos(int id, float& sx, float& sy) const;
    bool nodeInfo(int id, int& key, int& height, int& size, int& balance, int& depth) const;

    // Bahar se highlight control (level logic)
    void clearHighlights();
    void setMark(int id, int mark);
    void setSelected(int id);
    void setTarget(int id);

    int kind() const;
    bool isHeap() const;

    // Har node ke gird optional ornament. nullptr do to saada circle rahega.
    void setRingTexture(const sf::Texture* tex);

  private:
    Renderer(const Renderer&);
    Renderer& operator=(const Renderer&);

    const sf::Texture* m_ringTex;
    sf::Sprite m_ringSprite;

    void resetNode(RenderNode& r) const;
    void ensureCapacity(int n);
    sf::Text* makeText(unsigned int size) const;
    void refreshLabel(int i);
    void updateRegionLabels();
    void setActiveSize(int n);
    void markSubtree(int id, int mark, int depth);
    void startSwap(int a, int b);
    void finishSwap();
    void rebuildEdges();

    void drawNode(sf::RenderWindow& w, int i);
    void drawEffects(sf::RenderWindow& w);
    void drawArray(sf::RenderWindow& w, float left, float right);
    void drawTravelLabel(sf::RenderWindow& w, const Point& a, const Point& b);

    bool inCanvas(float sx, float sy) const;
    void screenToWorld(float sx, float sy, float& wx, float& wy) const;
    bool validIdx(int i) const;

    const sf::Font* m_font;
    float m_rx, m_ry, m_rw, m_rh;

    LayoutEngine m_layout;
    RenderNode* m_nodes;
    int m_cap;
    int m_count;
    int m_kind;
    int m_activeSize;

    sf::Text** m_key;
    sf::Text** m_badge;
    sf::Text** m_cell;
    sf::Text** m_idx;
    sf::Text* m_swapTxt[2];
    sf::Text* m_regionA;
    sf::Text* m_regionB;

    sf::CircleShape m_circle;
    sf::CircleShape m_ring;
    sf::RectangleShape m_rect;
    sf::VertexArray m_edges;
    bool m_edgesDirty;

    float m_tween;
    float m_tweenDur;

    // compare arrow
    int m_arrowNode;
    int m_arrowDir;
    float m_arrowT;
    sf::ConvexShape m_arrowHead;
    sf::Text* m_arrowTxt;

    // rotation label
    int m_rotNode;
    int m_rotType;
    float m_rotT;
    sf::Text* m_rotTxt;

    float m_zoom, m_cx, m_cy;
    float m_tZoom, m_tCx, m_tCy;
    bool m_userMoved;
    bool m_hasFit;
    bool m_dragging;
    int m_lastMx, m_lastMy;

    int m_swapA, m_swapB;
    float m_swapT;
};