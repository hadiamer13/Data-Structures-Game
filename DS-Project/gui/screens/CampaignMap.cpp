// gui/screens/CampaignMap.cpp
#include "CampaignMap.h"
#include "../UIManager.h"
#include "../Theme.h"
#include "../FakeCampaignData.h"   // INTEGRATION: remove at integration
#include <cstdio>

namespace {

    float maxF(float a, float b) { return a > b ? a : b; }
    float minF(float a, float b) { return a < b ? a : b; }

    // Index 0..3 = stars label, 4 = locked (no std::string needed).
    const char* const StatusText[5] = {
        "Not played yet", "Best: 1 star", "Best: 2 stars", "Best: 3 stars",
        "Locked - finish the previous level"
    };

    void centerOn(sf::Text& t, float x, float y) {
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
        t.setPosition(x, y);
    }

}  // namespace

CampaignMap::CampaignMap(AppContext& ctx) : m_ctx(ctx) {
    const sf::Font& font = ctx.assets->font();
    const float W = static_cast<float>(Theme::WindowW);
    const float H = static_cast<float>(Theme::WindowH);

    // ---- Background -------------------------------------------------
    // TODO(YOU): image path is Theme::CampaignBgPath (set in Theme.h,
    // loaded in Assets.cpp). Nothing to change here.
    const sf::Texture& bg = ctx.assets->campaignBackground();
    m_background.setTexture(bg);
    m_background.setScale(W / static_cast<float>(bg.getSize().x),
        H / static_cast<float>(bg.getSize().y));
    m_dim.setSize(sf::Vector2f(W, H));
    m_dim.setFillColor(sf::Color(0, 0, 0, 90));

    // ---- Layout numbers ----------------------------------------------
    // TODO(YOU): tweak these if your background art needs different spacing.
    const float topY = 15.f;
    const float bandH = 140.f;
    const float bandGap = 8.f;
    const float startX = W * 0.28f;     // first node x
    const float stepX = W * 0.155f;    // distance between nodes

    int linkIdx = 0;
    for (int a = 0; a < ActCount; ++a) {
        float bandTop = topY + a * (bandH + bandGap);
        float cy = bandTop + 70.f;

        // Act banner
        m_banner[a].setSize(sf::Vector2f(W - 40.f, bandH));
        m_banner[a].setPosition(20.f, bandTop);
        m_banner[a].setFillColor(Theme::ActColors[a]);
        m_banner[a].setOutlineColor(Theme::NodeLine);
        m_banner[a].setOutlineThickness(2.f);

        // Act title (two lines: "ACT n" / name)
        m_actTitle[a].setFont(font);
        m_actTitle[a].setString(FakeCampaign::ActTitles[a]);
        m_actTitle[a].setCharacterSize(24);
        m_actTitle[a].setFillColor(Theme::TitleColor);
        m_actTitle[a].setOutlineColor(sf::Color(0, 0, 0, 200));
        m_actTitle[a].setOutlineThickness(2.f);
        m_actTitle[a].setPosition(40.f, bandTop + 40.f);

        for (int l = 0; l < PerAct; ++l) {
            int idx = a * PerAct + l;
            float cx = startX + l * stepX;

            // Label like "2.3" built without std::string.
            char label[8];
            std::snprintf(label, sizeof(label), "%d.%d", a + 1, l + 1);

            // Boss levels: 2.5, 3.5, 4.5 (Act 1 has no boss).
            bool isBoss = (a >= 1 && l == PerAct - 1);

            m_nodes[idx].init(label, font, sf::Vector2f(cx, cy),
                Theme::NodeRadius, isBoss);
            m_nodeCtx[idx].map = this;
            m_nodeCtx[idx].index = idx;
            m_nodes[idx].setCallback(&CampaignMap::onNodeClick, &m_nodeCtx[idx]);

            // Link to the next node in the same act.
            if (l < PerAct - 1) {
                float x0 = cx + Theme::NodeRadius;
                float x1 = cx + stepX - Theme::NodeRadius;
                m_link[linkIdx].setSize(sf::Vector2f(x1 - x0, 4.f));
                m_link[linkIdx].setOrigin(0.f, 2.f);
                m_link[linkIdx].setPosition(x0, cy);
                ++linkIdx;
            }
        }
    }

    // ---- Back button --------------------------------------------------
    m_back.init("Back", font, sf::Vector2f(190.f, H - 55.f),
        sf::Vector2f(Theme::ButtonW * 0.6f, Theme::ButtonH));
    m_back.setCallback(&CampaignMap::onBackClick, this);

    // ---- Tooltip -------------------------------------------------------
    m_tipBox.setFillColor(Theme::TooltipBg);
    m_tipBox.setOutlineColor(Theme::NodeLine);
    m_tipBox.setOutlineThickness(2.f);

    m_tipTitle.setFont(font);
    m_tipTitle.setCharacterSize(22);
    m_tipTitle.setFillColor(Theme::TitleColor);
    m_tipConcept.setFont(font);
    m_tipConcept.setCharacterSize(Theme::TooltipTextSize);
    m_tipConcept.setFillColor(Theme::TextColor);
    m_tipStatus.setFont(font);
    m_tipStatus.setCharacterSize(Theme::TooltipTextSize);
    m_tipStatus.setFillColor(Theme::NodeGlow);

    // ---- Toast ----------------------------------------------------------
    m_toast.setFont(font);
    m_toast.setCharacterSize(Theme::ButtonTextSize);
    m_toast.setFillColor(Theme::TextColor);
    m_toast.setOutlineColor(sf::Color(0, 0, 0, 220));
    m_toast.setOutlineThickness(3.f);

    refreshFromData();
}

void CampaignMap::refreshFromData() {
    // ---- INTEGRATION POINT ------------------------------------------
    // Replace FakeCampaign::Stars / Unlocked with SaveManager results,
    // e.g. save->starsFor(idx), levelManager->isUnlocked(idx).
    // -----------------------------------------------------------------
    for (int i = 0; i < LevelCount; ++i)
        m_nodes[i].setState(FakeCampaign::Stars[i], FakeCampaign::Unlocked[i] != 0);

    // Link colour: gold if the next node is playable, grey if locked.
    int linkIdx = 0;
    for (int a = 0; a < ActCount; ++a) {
        for (int l = 0; l < PerAct - 1; ++l) {
            int next = a * PerAct + l + 1;
            bool open = FakeCampaign::Unlocked[next] != 0;
            m_link[linkIdx].setFillColor(open ? Theme::NodeLine
                : Theme::NodeLockedLine);
            ++linkIdx;
        }
    }
}

void CampaignMap::onNodeClick(void* context) {
    NodeCtx* c = static_cast<NodeCtx*>(context);
    c->map->onNodeChosen(c->index);
}

void CampaignMap::onBackClick(void* context) {
    static_cast<CampaignMap*>(context)->onBack();
}

void CampaignMap::onBack() {
    m_ctx.ui->requestScreen(ScreenId::MainMenu);
}

void CampaignMap::onNodeChosen(int index) {
    // ---- INTEGRATION POINT ------------------------------------------
    // Later: store act = index / 5, level = index % 5 and call
    //   m_ctx.ui->requestScreen(ScreenId::Briefing);
    // The Briefing screen then reads LevelDef from LevelLoader and
    // starts GameSession::startLevel(...).
    // -----------------------------------------------------------------
    char msg[64];
    std::snprintf(msg, sizeof(msg), "Selected %d.%d - briefing not built yet",
        index / PerAct + 1, index % PerAct + 1);
    m_toast.setString(msg);
    centerOn(m_toast, Theme::WindowW / 2.f, Theme::WindowH - 55.f);
    m_toastTime = 2.5f;
}

void CampaignMap::layoutTooltip(int idx) {
    m_tipTitle.setString(FakeCampaign::Names[idx]);
    m_tipConcept.setString(FakeCampaign::Concepts[idx]);
    int statusIdx = (FakeCampaign::Unlocked[idx] != 0)
        ? FakeCampaign::Stars[idx] : 4;
    m_tipStatus.setString(StatusText[statusIdx]);

    float w = maxF(m_tipTitle.getLocalBounds().width,
        maxF(m_tipConcept.getLocalBounds().width,
            m_tipStatus.getLocalBounds().width)) + 28.f;
    const float h = 100.f;
    m_tipBox.setSize(sf::Vector2f(w, h));

    // Prefer above the node; flip below if there is no room.
    const LevelNode& n = m_nodes[idx];
    float x = n.center().x - w / 2.f;
    float y = n.center().y - n.radius() - 40.f - h;
    if (y < 10.f) y = n.center().y + n.radius() + 40.f;

    // Keep inside the window.
    const float W = static_cast<float>(Theme::WindowW);
    x = maxF(10.f, minF(x, W - w - 10.f));

    m_tipBox.setPosition(x, y);
    m_tipTitle.setPosition(x + 14.f, y + 10.f);
    m_tipConcept.setPosition(x + 14.f, y + 44.f);
    m_tipStatus.setPosition(x + 14.f, y + 68.f);
}

void CampaignMap::handleEvent(const sf::Event& e) {
    // Esc is handled globally by UIManager.
    for (int i = 0; i < LevelCount; ++i) m_nodes[i].handleEvent(e);
    m_back.handleEvent(e);
}

void CampaignMap::update(float dt) {
    for (int i = 0; i < LevelCount; ++i) m_nodes[i].update(dt);
    m_back.update(dt);

    // Find the hovered node; only touch the strings when it changes.
    int hover = -1;
    for (int i = 0; i < LevelCount; ++i) {
        if (m_nodes[i].isHovered()) { hover = i; break; }
    }
    if (hover != m_hoverIdx) {
        m_hoverIdx = hover;
        if (hover >= 0) layoutTooltip(hover);
    }

    if (m_toastTime > 0.f) m_toastTime -= dt;
}

void CampaignMap::draw(sf::RenderWindow& window) {
    window.draw(m_background);
    window.draw(m_dim);

    for (int a = 0; a < ActCount; ++a) {
        window.draw(m_banner[a]);
        window.draw(m_actTitle[a]);
    }
    for (int i = 0; i < LinkCount; ++i) window.draw(m_link[i]);
    for (int i = 0; i < LevelCount; ++i) m_nodes[i].draw(window);

    m_back.draw(window);
    if (m_toastTime > 0.f) window.draw(m_toast);

    // Tooltip last so it sits on top of everything.
    if (m_hoverIdx >= 0) {
        window.draw(m_tipBox);
        window.draw(m_tipTitle);
        window.draw(m_tipConcept);
        window.draw(m_tipStatus);
    }
}