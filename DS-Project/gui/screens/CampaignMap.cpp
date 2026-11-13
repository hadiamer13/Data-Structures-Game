// gui/screens/CampaignMap.cpp
#include "CampaignMap.h"
#include "../UIManager.h"
#include "../Theme.h"
#include "../FakeCampaignData.h"   // INTEGRATION: remove at integration
#include <cstdio>
#include <cmath>

namespace {

    float minF(float a, float b) { return a < b ? a : b; }
    float maxF(float a, float b) { return a > b ? a : b; }

    // Act names alone (FakeCampaign::ActTitles has two lines, not suited here).
    const char* const ActNames[4] = {
        "The Blueprint Wing", "The Sorted Wing",
        "The Architect's Wing", "The Heap Fortress"
    };

    void centerOrigin(sf::Text& t) {
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    }

}  // namespace

void CampaignMap::styleText(sf::Text& t, const sf::Font& font, unsigned size,
    const sf::Color& col, float x, float y) {
    t.setFont(font);                          // SFML 2.x API
    t.setCharacterSize(size);
    t.setFillColor(col);
    t.setOutlineColor(sf::Color(0, 0, 0, 200));
    t.setOutlineThickness(2.f);
    t.setPosition(x, y);
}

void CampaignMap::buildArrow(sf::ConvexShape& s, bool left, float cx, float cy) {
    s.setPointCount(3);
    float d = left ? -1.f : 1.f;
    s.setPoint(0, sf::Vector2f(d * 14.f, 0.f));      // tip
    s.setPoint(1, sf::Vector2f(-d * 10.f, -16.f));
    s.setPoint(2, sf::Vector2f(-d * 10.f, 16.f));
    s.setPosition(cx, cy);
    s.setOutlineThickness(2.f);
}

void CampaignMap::buildStars(float x, float y) {
    const float outer = 14.f;
    const float inner = outer * 0.45f;
    for (int s = 0; s < 3; ++s) {
        m_star[s].setPointCount(10);
        for (int p = 0; p < 10; ++p) {
            float r = (p % 2 == 0) ? outer : inner;
            float a = -Theme::Pi / 2.f + p * (Theme::Pi / 5.f);
            m_star[s].setPoint(p, sf::Vector2f(r * std::cos(a), r * std::sin(a)));
        }
        m_star[s].setPosition(x + s * 36.f, y);
        m_star[s].setOutlineThickness(1.5f);
    }
}

CampaignMap::CampaignMap(AppContext& ctx) : m_ctx(ctx) {
    const sf::Font& font = ctx.assets->font();
    const float W = static_cast<float>(Theme::WindowW);
    const float H = static_cast<float>(Theme::WindowH);

    m_arrowAmt[0] = m_arrowAmt[1] = 0.f;
    m_arrowHover[0] = m_arrowHover[1] = false;

    // ---- Background -------------------------------------------------
    // TODO(YOU): image path is Theme::CampaignBgPath (Theme.h). A dark
    // red background suits this screen best.
    const sf::Texture& bg = ctx.assets->campaignBackground();
    m_background.setTexture(bg);
    m_background.setScale(W / static_cast<float>(bg.getSize().x),
        H / static_cast<float>(bg.getSize().y));
    m_dim.setSize(sf::Vector2f(W, H));
    m_dim.setFillColor(sf::Color(0, 0, 0, 110));

    // ---- Corner decoration (4 L shapes made of 2 bars each) ----------
    // TODO(YOU): swap for your corner image from the pack if you want;
    // load it in Assets and draw a sprite per corner instead.
    {
        const float inset = 12.f, len = 70.f, th = 3.f;
        const float xs[4] = { inset, W - inset, inset, W - inset };
        const float ys[4] = { inset, inset, H - inset, H - inset };
        const float dx[4] = { 1.f, -1.f, 1.f, -1.f };
        const float dy[4] = { 1.f, 1.f, -1.f, -1.f };
        for (int c = 0; c < 4; ++c) {
            sf::RectangleShape& h = m_corner[c * 2];
            sf::RectangleShape& v = m_corner[c * 2 + 1];
            h.setSize(sf::Vector2f(len * dx[c], th));
            h.setPosition(xs[c], ys[c] - (dy[c] > 0 ? 0.f : th));
            v.setSize(sf::Vector2f(th, len * dy[c]));
            v.setPosition(xs[c] - (dx[c] > 0 ? 0.f : th), ys[c]);
            h.setFillColor(Theme::NodeLine);
            v.setFillColor(Theme::NodeLine);
        }
    }

    // ---- Top bar: tabs ------------------------------------------------
    const float tabY = 30.f;
    for (int a = 0; a < ActCount; ++a) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "ACT %d", a + 1);
        float cx = W / 2.f + (a - 1.5f) * 190.f;
        styleText(m_tab[a], font, 26, Theme::TextColor, cx, tabY);
        m_tab[a].setString(buf);
        centerOrigin(m_tab[a]);
        m_tab[a].setPosition(cx, tabY);
        m_tabRect[a] = sf::FloatRect(cx - 80.f, tabY - 20.f, 160.f, 40.f);
    }
    m_tabUnder.setSize(sf::Vector2f(110.f, 3.f));
    m_tabUnder.setFillColor(Theme::NodeGlow);

    // Arrows sit just outside the first and last tab.
    float lx = W / 2.f - 1.5f * 190.f - 130.f;
    float rx = W / 2.f + 1.5f * 190.f + 130.f;
    buildArrow(m_arrow[0], true, lx, tabY);
    buildArrow(m_arrow[1], false, rx, tabY);
    m_arrowRect[0] = sf::FloatRect(lx - 25.f, tabY - 25.f, 50.f, 50.f);
    m_arrowRect[1] = sf::FloatRect(rx - 25.f, tabY - 25.f, 50.f, 50.f);

    styleText(m_actName, font, 18, Theme::NodeGlow, W / 2.f, 64.f);

    // Divider with a diamond in the middle.
    m_divider.setSize(sf::Vector2f(700.f, 2.f));
    m_divider.setOrigin(350.f, 1.f);
    m_divider.setPosition(W / 2.f, 82.f);
    m_divider.setFillColor(Theme::NodeLine);
    m_diamond.setPointCount(4);
    m_diamond.setPoint(0, sf::Vector2f(0.f, -7.f));
    m_diamond.setPoint(1, sf::Vector2f(7.f, 0.f));
    m_diamond.setPoint(2, sf::Vector2f(0.f, 7.f));
    m_diamond.setPoint(3, sf::Vector2f(-7.f, 0.f));
    m_diamond.setPosition(W / 2.f, 82.f);
    m_diamond.setFillColor(Theme::NodeGlow);

    // ---- Banners ----------------------------------------------------
    // TODO(YOU): if your banner frame looks too wide/narrow, change the
    // 215 px spacing below and Theme::BannerH.
    const float bannerY = 262.f;
    for (int i = 0; i < PerAct; ++i) {
        char num[8];
        std::snprintf(num, sizeof(num), "%d", i + 1);
        m_banner[i].init(num, font, ctx.assets->banner(), ctx.assets->skull(),
            sf::Vector2f(W / 2.f + (i - 2) * 215.f, bannerY),
            Theme::BannerH, false);   // boss flag set in setAct
        m_bannerCtx[i].map = this;
        m_bannerCtx[i].slot = i;
        m_banner[i].setCallback(&CampaignMap::onBannerClick, &m_bannerCtx[i]);
    }

    // ---- Info panel (under the banners) -------------------------------
    m_panelX = 100.f;  m_panelY = 448.f;
    m_panelW = W - 200.f;  m_panelH = 252.f;
    m_panel.setSize(sf::Vector2f(m_panelW, m_panelH));
    m_panel.setPosition(m_panelX, m_panelY);
    m_panel.setFillColor(Theme::TooltipBg);
    m_panel.setOutlineColor(Theme::NodeLine);
    m_panel.setOutlineThickness(3.f);

    const float tx = m_panelX + 30.f;
    styleText(m_title, font, 34, Theme::TitleColor, tx, m_panelY + 14.f);
    styleText(m_status, font, 18, Theme::NodeGlow, tx, m_panelY + 60.f);
    styleText(m_concept, font, 20, Theme::TextColor, tx, m_panelY + 92.f);
    styleText(m_objective, font, 20, Theme::TextColor, tx, m_panelY + 126.f);

    const float rx2 = m_panelX + m_panelW - 280.f;
    styleText(m_bestLabel, font, 18, Theme::NodeGlow, rx2, m_panelY + 60.f);
    buildStars(rx2 + 90.f, m_panelY + 71.f);
    styleText(m_lockedMsg, font, 18, Theme::NodeLockedLine, rx2, m_panelY + 150.f);
    m_lockedMsg.setString("LOCKED\nFinish the previous level\nto unlock this one.");

    // ---- Buttons ----------------------------------------------------------
    m_start.init("Start", font,
        sf::Vector2f(m_panelX + m_panelW - 150.f, m_panelY + m_panelH - 55.f),
        sf::Vector2f(240.f, Theme::ButtonH));
    m_start.setCallback(&CampaignMap::onStartClick, this);

    m_back.init("Back", font, sf::Vector2f(110.f, tabY),
        sf::Vector2f(150.f, 42.f));
    m_back.setCallback(&CampaignMap::onBackClick, this);

    // ---- Restore position when coming back from the Briefing -----------------
    int act = 0, sel = 0;
    if (ctx.selectedLevel >= 0 && ctx.selectedLevel < ActCount * PerAct) {
        act = ctx.selectedLevel / PerAct;
        sel = ctx.selectedLevel % PerAct;
    }
    m_act = act;
    m_sel = sel;
    setAct(act);
    setSelected(sel);
}

void CampaignMap::loadBannerStates() {
    // ---- INTEGRATION POINT ------------------------------------------
    // Replace FakeCampaign::Stars / Unlocked with SaveManager and
    // LevelManager results, e.g. save->starsFor(i), lm->isUnlocked(i).
    // -----------------------------------------------------------------
    for (int s = 0; s < PerAct; ++s) {
        int i = m_act * PerAct + s;
        m_banner[s].setState(FakeCampaign::Stars[i], FakeCampaign::Unlocked[i] != 0);
    }
}

void CampaignMap::refreshFromData() {
    loadBannerStates();
    refreshInfo();
}

void CampaignMap::setAct(int act) {
    if (act < 0) act = 0;
    if (act >= ActCount) act = ActCount - 1;
    m_act = act;

    // Boss flag: slot 4 of acts 2..4. LevelBanner::init set it to false,
    // so re-init the four banners that can be a boss only via a fresh init.
    const sf::Font& font = *m_status.getFont();
    const float W = static_cast<float>(Theme::WindowW);
    for (int i = 0; i < PerAct; ++i) {
        char num[8];
        std::snprintf(num, sizeof(num), "%d", i + 1);
        bool boss = (act >= 1 && i == PerAct - 1);
        m_banner[i].init(num, font, m_ctx.assets->banner(), m_ctx.assets->skull(),
            sf::Vector2f(W / 2.f + (i - 2) * 215.f, 262.f),
            Theme::BannerH, boss);
    }
    loadBannerStates();

    // Tab visuals
    for (int a = 0; a < ActCount; ++a)
        m_tab[a].setFillColor(a == act ? Theme::NodeGlow : Theme::TextColor);
    m_tabUnder.setOrigin(55.f, 1.5f);
    m_tabUnder.setPosition(m_tab[act].getPosition().x,
        m_tab[act].getPosition().y + 24.f);

    m_actName.setString(ActNames[act]);
    centerOrigin(m_actName);
    m_actName.setPosition(W / 2.f, 64.f);

    // Select the first level still to play (or the first one).
    int pick = 0;
    for (int s = 0; s < PerAct; ++s) {
        int i = act * PerAct + s;
        if (FakeCampaign::Unlocked[i] != 0 && FakeCampaign::Stars[i] == 0) {
            pick = s;
            break;
        }
    }
    setSelected(pick);
}

void CampaignMap::setSelected(int slot) {
    if (slot < 0) slot = 0;
    if (slot >= PerAct) slot = PerAct - 1;
    m_sel = slot;
    for (int s = 0; s < PerAct; ++s) m_banner[s].setSelected(s == slot);
    refreshInfo();
}

void CampaignMap::refreshInfo() {
    // Strings are only rewritten here (on change), never per frame.
    int i = levelIndex();
    bool unlocked = FakeCampaign::Unlocked[i] != 0;
    int stars = unlocked ? FakeCampaign::Stars[i] : 0;
    bool boss = (m_act >= 1 && m_sel == PerAct - 1);

    m_title.setString(FakeCampaign::Names[i]);
    m_concept.setString(FakeCampaign::Concepts[i]);
    m_objective.setString(FakeCampaign::Objectives[i]);

    // Not colour alone: status is a word.
    const char* word = "OPEN";
    if (!unlocked)      word = "LOCKED";
    else if (stars > 0) word = boss ? "CLEARED  -  BOSS LEVEL" : "CLEARED";
    else if (boss)      word = "OPEN  -  BOSS LEVEL";
    m_status.setString(word);

    m_bestLabel.setString("BEST");
    for (int s = 0; s < 3; ++s) {
        bool on = s < stars;
        m_star[s].setFillColor(on ? Theme::StarOn : sf::Color(0, 0, 0, 0));
        m_star[s].setOutlineColor(on ? Theme::StarOn : Theme::StarOff);
    }
}

void CampaignMap::onBannerClick(void* context) {
    BannerCtx* c = static_cast<BannerCtx*>(context);
    c->map->onBannerClicked(c->slot);
}
void CampaignMap::onStartClick(void* context) {
    static_cast<CampaignMap*>(context)->onStart();
}
void CampaignMap::onBackClick(void* context) {
    static_cast<CampaignMap*>(context)->onBack();
}

void CampaignMap::onBannerClicked(int slot) {
    // First click selects; clicking the selected banner opens the Briefing.
    if (slot == m_sel) onStart();
    else               setSelected(slot);
}

void CampaignMap::onStart() {
    int i = levelIndex();
    if (FakeCampaign::Unlocked[i] == 0) return;      // locked: nothing happens
    // ---- INTEGRATION POINT ------------------------------------------
    // Later: pass a LevelDef (from LevelLoader) instead of a bare index.
    // -----------------------------------------------------------------
    m_ctx.selectedLevel = i;
    m_ctx.ui->requestScreen(ScreenId::Briefing);
}

void CampaignMap::onBack() {
    m_ctx.ui->requestScreen(ScreenId::MainMenu);
}

void CampaignMap::handleEvent(const sf::Event& e) {
    // Esc is handled globally by UIManager.
    if (e.type == sf::Event::KeyPressed) {
        sf::Keyboard::Key k = e.key.code;
        if (k == sf::Keyboard::Left || k == sf::Keyboard::Q) setAct(m_act - 1);
        if (k == sf::Keyboard::Right || k == sf::Keyboard::E) setAct(m_act + 1);
        if (k == sf::Keyboard::Return) onStart();
    }

    if (e.type == sf::Event::MouseMoved) {
        float mx = static_cast<float>(e.mouseMove.x);
        float my = static_cast<float>(e.mouseMove.y);
        m_tabHover = -1;
        for (int a = 0; a < ActCount; ++a)
            if (m_tabRect[a].contains(mx, my)) m_tabHover = a;
        for (int s = 0; s < 2; ++s)
            m_arrowHover[s] = m_arrowRect[s].contains(mx, my);
    }

    if (e.type == sf::Event::MouseButtonPressed &&
        e.mouseButton.button == sf::Mouse::Left) {
        float mx = static_cast<float>(e.mouseButton.x);
        float my = static_cast<float>(e.mouseButton.y);
        if (m_arrowRect[0].contains(mx, my)) setAct(m_act - 1);
        if (m_arrowRect[1].contains(mx, my)) setAct(m_act + 1);
        for (int a = 0; a < ActCount; ++a)
            if (m_tabRect[a].contains(mx, my)) setAct(a);
    }

    for (int s = 0; s < PerAct; ++s) m_banner[s].handleEvent(e);
    m_back.handleEvent(e);
    if (FakeCampaign::Unlocked[levelIndex()] != 0) m_start.handleEvent(e);
}

void CampaignMap::update(float dt) {
    for (int s = 0; s < PerAct; ++s) m_banner[s].update(dt);
    m_back.update(dt);
    m_start.update(dt);

    // Tab hover: brighten (current act stays highlighted).
    for (int a = 0; a < ActCount; ++a) {
        if (a == m_act) m_tab[a].setFillColor(Theme::NodeGlow);
        else if (a == m_tabHover) m_tab[a].setFillColor(Theme::BannerHover);
        else m_tab[a].setFillColor(Theme::TextColor);
    }

    // Arrows: ease the hover scale; dim at the ends (not colour alone:
    // an arrow at the end is also drawn smaller and without outline).
    for (int s = 0; s < 2; ++s) {
        float t = m_arrowHover[s] ? 1.f : 0.f;
        float st = 8.f * dt;
        if (m_arrowAmt[s] < t) m_arrowAmt[s] = minF(t, m_arrowAmt[s] + st);
        else                   m_arrowAmt[s] = maxF(t, m_arrowAmt[s] - st);

        bool atEnd = (s == 0 && m_act == 0) || (s == 1 && m_act == ActCount - 1);
        float sc = (atEnd ? 0.7f : 1.f) + 0.25f * m_arrowAmt[s];
        m_arrow[s].setScale(sc, sc);
        sf::Color fill = atEnd ? Theme::StarOff : Theme::NodeLine;
        if (!atEnd && m_arrowAmt[s] > 0.5f) fill = Theme::NodeGlow;
        m_arrow[s].setFillColor(fill);
        m_arrow[s].setOutlineColor(atEnd ? sf::Color(0, 0, 0, 0)
            : sf::Color(0, 0, 0, 200));
    }
}

void CampaignMap::draw(sf::RenderWindow& window) {
    window.draw(m_background);
    window.draw(m_dim);
    for (int c = 0; c < 8; ++c) window.draw(m_corner[c]);

    // Top bar
    m_back.draw(window);
    for (int a = 0; a < ActCount; ++a) window.draw(m_tab[a]);
    window.draw(m_tabUnder);
    window.draw(m_arrow[0]);
    window.draw(m_arrow[1]);
    window.draw(m_actName);
    window.draw(m_divider);
    window.draw(m_diamond);

    // Banners: selected one last so its glow and scale sit on top.
    for (int s = 0; s < PerAct; ++s)
        if (s != m_sel) m_banner[s].draw(window);
    m_banner[m_sel].draw(window);

    // Info panel
    window.draw(m_panel);
    window.draw(m_title);
    window.draw(m_status);
    window.draw(m_concept);
    window.draw(m_objective);
    window.draw(m_bestLabel);
    for (int s = 0; s < 3; ++s) window.draw(m_star[s]);

    if (FakeCampaign::Unlocked[levelIndex()] != 0) m_start.draw(window);
    else                                           window.draw(m_lockedMsg);
}