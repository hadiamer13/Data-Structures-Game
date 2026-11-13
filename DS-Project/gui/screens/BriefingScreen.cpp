// gui/screens/BriefingScreen.cpp
#include "BriefingScreen.h"
#include "../UIManager.h"
#include "../Theme.h"
#include "../FakeCampaignData.h"   // INTEGRATION: remove at integration
#include <cstdio>

void BriefingScreen::makeLabel(sf::Text& t, const sf::Font& font,
    const char* str, unsigned size,
    const sf::Color& col, float x, float y) {
    t.setFont(font);                       // SFML 2.x API
    t.setString(str);
    t.setCharacterSize(size);
    t.setFillColor(col);
    t.setPosition(x, y);
}

void BriefingScreen::buildStars(float x, float y) {
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
        bool on = s < m_stars;
        m_star[s].setFillColor(on ? Theme::StarOn : sf::Color(0, 0, 0, 0));
        m_star[s].setOutlineColor(on ? Theme::StarOn : Theme::StarOff);
    }
}

BriefingScreen::BriefingScreen(AppContext& ctx) : m_ctx(ctx) {
    const sf::Font& font = ctx.assets->font();
    const float W = static_cast<float>(Theme::WindowW);
    const float H = static_cast<float>(Theme::WindowH);

    // Which level was chosen on the map.
    m_index = ctx.selectedLevel;
    if (m_index < 0 || m_index > 19) m_index = 0;

    // ---- INTEGRATION POINT ------------------------------------------
    // Replace the FakeCampaign reads below with the LevelDef from
    // LevelLoader (name, concept, objective) and SaveManager (best stars).
    // -----------------------------------------------------------------
    m_stars = FakeCampaign::Stars[m_index];
    int act = m_index / 5;
    int lvl = m_index % 5;
    bool isBoss = (act >= 1 && lvl == 4);

    // ---- Background ----------------------------------------------------
    // TODO(YOU): reuses the campaign background. For a dedicated briefing
    // image, add BriefingBgPath to Theme.h and a texture to Assets, then
    // swap the texture here.
    const sf::Texture& bg = ctx.assets->campaignBackground();
    m_background.setTexture(bg);
    m_background.setScale(W / static_cast<float>(bg.getSize().x),
        H / static_cast<float>(bg.getSize().y));
    m_dim.setSize(sf::Vector2f(W, H));
    m_dim.setFillColor(sf::Color(0, 0, 0, 0));   // fades in via update()

    // ---- Panel -----------------------------------------------------------
    const float pw = 860.f, ph = 540.f;
    const float px = (W - pw) / 2.f, py = 90.f;
    m_panel.setSize(sf::Vector2f(pw, ph));
    m_panel.setPosition(px, py);
    m_panel.setFillColor(Theme::TooltipBg);
    m_panel.setOutlineColor(Theme::ActColors[act]);
    m_panel.setOutlineThickness(4.f);

    const float lx = px + 40.f;

    char buf[64];
    std::snprintf(buf, sizeof(buf), "ACT %d  -  LEVEL %d.%d", act + 1, act + 1, lvl + 1);
    makeLabel(m_act, font, buf, 20, Theme::NodeGlow, lx, py + 22.f);

    makeLabel(m_title, font, FakeCampaign::Names[m_index], 40,
        Theme::TitleColor, lx, py + 50.f);

    // Boss tag: text, not colour alone.
    makeLabel(m_boss, font, "BOSS LEVEL", 20, Theme::StarOn,
        px + pw - 190.f, py + 24.f);

    m_divider.setSize(sf::Vector2f(pw - 80.f, 2.f));
    m_divider.setPosition(lx, py + 115.f);
    m_divider.setFillColor(Theme::NodeLine);

    // ---- Text blocks -----------------------------------------------------
    makeLabel(m_objHead, font, "OBJECTIVE", 18, Theme::NodeGlow, lx, py + 135.f);
    makeLabel(m_objective, font, FakeCampaign::Objectives[m_index], 24,
        Theme::TextColor, lx, py + 162.f);

    makeLabel(m_conceptHead, font, "CONCEPT", 18, Theme::NodeGlow, lx, py + 270.f);
    makeLabel(m_concept, font, FakeCampaign::Concepts[m_index], 22,
        Theme::TextColor, lx, py + 297.f);

    // INTEGRATION: the real capacity (ceil(1.5 x optimal) + 2) comes from
    // the model via LevelManager. Show it here once the session exists.
    makeLabel(m_ruleHead, font, "ALARM RULE", 18, Theme::NodeGlow, lx, py + 345.f);
    makeLabel(m_rule, font,
        "Every room visited or key compared costs 1 alarm step.\n"
        "When the alarm is full, the level is failed.",
        18, Theme::TextColor, lx, py + 372.f);

    makeLabel(m_bestLabel, font, "BEST", 18, Theme::NodeGlow, lx, py + 430.f);
    buildStars(lx + 110.f, py + 441.f);

    // ---- Buttons -----------------------------------------------------------
    const float by = py + ph - 45.f;
    m_backBtn.init("Back", font, sf::Vector2f(W / 2.f - 180.f, by),
        sf::Vector2f(Theme::ButtonW * 0.8f, Theme::ButtonH));
    m_backBtn.setCallback(&BriefingScreen::onBackClick, this);

    m_start.init("Start", font, sf::Vector2f(W / 2.f + 180.f, by),
        sf::Vector2f(Theme::ButtonW * 0.8f, Theme::ButtonH));
    m_start.setCallback(&BriefingScreen::onStartClick, this);
}

void BriefingScreen::onStartClick(void* context) {
    static_cast<BriefingScreen*>(context)->onStart();
}
void BriefingScreen::onBackClick(void* context) {
    static_cast<BriefingScreen*>(context)->onBack();
}

void BriefingScreen::onBack() {
    m_ctx.ui->requestScreen(ScreenId::Campaign);
}

void BriefingScreen::onStart() {
    // ---- INTEGRATION POINT ------------------------------------------
    // Later: call m_ctx.session->startLevel(levelDef) here, then switch.
    // Act 4 levels will go to a HeapLevel screen instead of Level.
    // -----------------------------------------------------------------
    m_ctx.ui->requestScreen(ScreenId::Level);
}

void BriefingScreen::handleEvent(const sf::Event& e) {
    // Esc is handled by UIManager (returns to the Campaign Map).
    // Enter / Space also start the level.
    if (e.type == sf::Event::KeyPressed &&
        (e.key.code == sf::Keyboard::Return || e.key.code == sf::Keyboard::Space)) {
        onStart();
        return;
    }
    m_backBtn.handleEvent(e);
    m_start.handleEvent(e);
}

void BriefingScreen::update(float dt) {
    m_backBtn.update(dt);
    m_start.update(dt);

    if (m_fade < 1.f) {
        m_fade += dt * 4.f;                 // ~0.25 s fade-in
        if (m_fade > 1.f) m_fade = 1.f;
        m_dim.setFillColor(sf::Color(0, 0, 0,
            static_cast<sf::Uint8>(170.f * m_fade)));
    }
}

void BriefingScreen::draw(sf::RenderWindow& window) {
    window.draw(m_background);
    window.draw(m_dim);
    window.draw(m_panel);

    window.draw(m_act);
    window.draw(m_title);
    if (m_index % 5 == 4 && m_index >= 5) window.draw(m_boss);
    window.draw(m_divider);

    window.draw(m_objHead);
    window.draw(m_objective);
    window.draw(m_conceptHead);
    window.draw(m_concept);
    window.draw(m_ruleHead);
    window.draw(m_rule);

    window.draw(m_bestLabel);
    for (int s = 0; s < 3; ++s) window.draw(m_star[s]);

    m_backBtn.draw(window);
    m_start.draw(window);
}