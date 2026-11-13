// gui/widgets/LevelBanner.cpp
#include "LevelBanner.h"
#include "../Theme.h"
#include <cmath>

namespace {

    float minF(float a, float b) { return a < b ? a : b; }
    float maxF(float a, float b) { return a > b ? a : b; }

    sf::Uint8 mixChannel(sf::Uint8 x, sf::Uint8 y, float t) {
        return static_cast<sf::Uint8>(x + (y - x) * t);
    }
    sf::Color lerpColor(const sf::Color& a, const sf::Color& b, float t) {
        return sf::Color(mixChannel(a.r, b.r, t), mixChannel(a.g, b.g, t),
            mixChannel(a.b, b.b, t), mixChannel(a.a, b.a, t));
    }

    void centerOrigin(sf::Text& t) {
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    }

    void styleText(sf::Text& t, const sf::Font& font, unsigned size,
        const sf::Color& col) {
        t.setFont(font);                         // SFML 2.x API
        t.setCharacterSize(size);
        t.setFillColor(col);
        t.setOutlineColor(sf::Color(0, 0, 0, 200));
        t.setOutlineThickness(2.f);
    }

}  // namespace

void LevelBanner::init(const char* number, const sf::Font& font,
    const sf::Texture& banner, const sf::Texture& skull,
    sf::Vector2f center, float height, bool isBoss) {
    m_center = center;
    m_height = height;
    m_boss = isBoss;

    // Sprites: origin at the middle so scaling keeps the banner centred.
    sf::Vector2u bs = banner.getSize();
    m_banner.setTexture(banner);
    m_banner.setOrigin(bs.x / 2.f, bs.y / 2.f);
    m_glow.setTexture(banner);
    m_glow.setOrigin(bs.x / 2.f, bs.y / 2.f);

    sf::Vector2u ks = skull.getSize();
    m_skull.setTexture(skull);
    m_skull.setOrigin(ks.x / 2.f, ks.y / 2.f);
    m_skull.setColor(Theme::NodeGlow);

    styleText(m_head, font, 18, Theme::TextColor);
    m_head.setString("LEVEL");
    centerOrigin(m_head);

    styleText(m_num, font, 64, Theme::TitleColor);
    m_num.setString(number);
    centerOrigin(m_num);

    styleText(m_status, font, 14, Theme::TextColor);

    // Padlock built from shapes (no image needed).
    m_lockBody.setSize(sf::Vector2f(24.f, 18.f));
    m_lockBody.setOrigin(12.f, 9.f);
    m_lockBody.setFillColor(Theme::NodeLockedLine);
    m_lockShackle.setRadius(7.f);
    m_lockShackle.setOrigin(7.f, 7.f);
    m_lockShackle.setFillColor(sf::Color(0, 0, 0, 0));
    m_lockShackle.setOutlineColor(Theme::NodeLockedLine);
    m_lockShackle.setOutlineThickness(3.f);

    buildStars();
    setState(0, false);
    applyLayout(0.82f);
}

void LevelBanner::buildStars() {
    const float outer = 11.f;
    const float inner = outer * 0.45f;
    for (int s = 0; s < 3; ++s) {
        m_star[s].setPointCount(10);
        for (int p = 0; p < 10; ++p) {
            float r = (p % 2 == 0) ? outer : inner;
            float a = -Theme::Pi / 2.f + p * (Theme::Pi / 5.f);
            m_star[s].setPoint(p, sf::Vector2f(r * std::cos(a), r * std::sin(a)));
        }
        m_star[s].setOutlineThickness(1.5f);
    }
}

void LevelBanner::setState(int stars, bool unlocked) {
    m_unlocked = unlocked;
    m_stars = unlocked ? stars : 0;
    if (m_stars < 0) m_stars = 0;
    if (m_stars > 3) m_stars = 3;

    // Not colour alone: status word + padlock/stars.
    const char* word = "OPEN";
    if (!m_unlocked)       word = "LOCKED";
    else if (m_boss)       word = "BOSS";
    else if (m_stars > 0)  word = "CLEARED";
    m_status.setString(word);
    centerOrigin(m_status);

    for (int s = 0; s < 3; ++s) {
        bool on = s < m_stars;
        m_star[s].setFillColor(on ? Theme::StarOn : sf::Color(0, 0, 0, 0));
        m_star[s].setOutlineColor(on ? Theme::StarOn : Theme::StarOff);
    }

    m_num.setFillColor(m_unlocked ? Theme::TitleColor : Theme::NodeLockedLine);
}

void LevelBanner::applyLayout(float s) {
    const sf::Vector2u ts = m_banner.getTexture()->getSize();
    float base = (m_height / static_cast<float>(ts.y)) * s;

    m_banner.setScale(base, base);
    m_banner.setPosition(m_center);

    float gs = base * (1.05f + 0.015f * std::sin(m_time * 4.f));
    m_glow.setScale(gs, gs);
    m_glow.setPosition(m_center);

    // Offsets are fractions of the banner height, scaled by s.
    float H = m_height * s;
    float cx = m_center.x, cy = m_center.y;

    m_head.setScale(s, s);
    m_head.setPosition(cx, cy - 0.20f * H);
    m_num.setScale(s, s);
    m_num.setPosition(cx, cy - 0.08f * H);
    m_status.setScale(s, s);
    m_status.setPosition(cx, cy + 0.10f * H);

    m_skull.setScale(s * 0.9f, s * 0.9f);
    m_skull.setPosition(cx, cy - 0.33f * H);

    // Stars (cleared/unlocked) or padlock (locked) share the bottom slot.
    float by = cy + 0.24f * H;
    for (int i = 0; i < 3; ++i) {
        m_star[i].setScale(s, s);
        m_star[i].setPosition(cx + (i - 1) * 28.f * s, by);
    }
    m_lockBody.setScale(s, s);
    m_lockBody.setPosition(cx, by + 4.f * s);
    m_lockShackle.setScale(s, s);
    m_lockShackle.setPosition(cx, by - 8.f * s);
}

void LevelBanner::handleEvent(const sf::Event& e) {
    // NOTE: raw window pixel coordinates (same as Button).
    sf::FloatRect b = m_banner.getGlobalBounds();

    if (e.type == sf::Event::MouseMoved) {
        if (m_state != StatePressed)
            m_state = b.contains(static_cast<float>(e.mouseMove.x),
                static_cast<float>(e.mouseMove.y))
            ? StateHover : StateIdle;
    }
    else if (e.type == sf::Event::MouseButtonPressed) {
        if (e.mouseButton.button == sf::Mouse::Left &&
            b.contains(static_cast<float>(e.mouseButton.x),
                static_cast<float>(e.mouseButton.y)))
            m_state = StatePressed;
    }
    else if (e.type == sf::Event::MouseButtonReleased) {
        if (e.mouseButton.button == sf::Mouse::Left && m_state == StatePressed) {
            bool still = b.contains(static_cast<float>(e.mouseButton.x),
                static_cast<float>(e.mouseButton.y));
            m_state = still ? StateHover : StateIdle;
            // INTEGRATION: CampaignMap's callback decides: first click
            // selects, clicking the selected banner opens the Briefing.
            // Locked banners can be selected too (to read their info).
            if (still && m_onClick) m_onClick(m_context);
        }
    }
}

void LevelBanner::update(float dt) {
    m_time += dt;

    float ht = (m_state == StateIdle) ? 0.f : 1.f;
    float hs = 8.f * dt;
    if (m_hoverAmt < ht) m_hoverAmt = minF(ht, m_hoverAmt + hs);
    else                 m_hoverAmt = maxF(ht, m_hoverAmt - hs);

    float st = m_selected ? 1.f : 0.f;
    float ss = 7.f * dt;
    if (m_selAmt < st) m_selAmt = minF(st, m_selAmt + ss);
    else               m_selAmt = maxF(st, m_selAmt - ss);

    // Scale: 0.82 unselected, 1.0 selected, +0.04 on hover, 0.97x pressed.
    float s = 0.82f + 0.18f * m_selAmt + 0.04f * m_hoverAmt;
    if (m_state == StatePressed) s *= 0.97f;
    applyLayout(s);

    // Banner tint: grey when locked, red otherwise, brighter when selected.
    sf::Color base = m_unlocked ? Theme::BannerNormal : Theme::BannerLocked;
    sf::Color c = lerpColor(base, Theme::BannerSelected, m_selAmt);
    c = lerpColor(c, Theme::BannerHover, m_hoverAmt * 0.6f);
    m_banner.setColor(c);

    // Pulsing glow behind the banner (selected or hovered). Locked = dim.
    float strength = maxF(m_selAmt, m_hoverAmt);
    float pulse = 0.75f + 0.25f * std::sin(m_time * 4.f);
    sf::Color g = m_unlocked ? Theme::NodeGlow : Theme::NodeLockedLine;
    g.a = static_cast<sf::Uint8>((m_unlocked ? 150.f : 70.f) * strength * pulse);
    m_glow.setColor(g);
}

void LevelBanner::draw(sf::RenderWindow& window) const {
    if (m_hoverAmt > 0.01f || m_selAmt > 0.01f) window.draw(m_glow);
    window.draw(m_banner);

    if (m_boss) window.draw(m_skull);
    window.draw(m_head);
    window.draw(m_num);
    window.draw(m_status);

    if (!m_unlocked) {
        window.draw(m_lockShackle);
        window.draw(m_lockBody);
    }
    else {
        for (int s = 0; s < 3; ++s) window.draw(m_star[s]);   // empty = outline
    }
}