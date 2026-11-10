// gui/widgets/LevelNode.cpp
#include "LevelNode.h"
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

}  // namespace

void LevelNode::init(const char* label, const sf::Font& font,
    sf::Vector2f center, float radius, bool isBoss) {
    m_center = center;
    m_radius = radius;
    m_boss = isBoss;

    m_body.setOutlineThickness(3.f);
    m_glow.setFillColor(sf::Color(0, 0, 0, 0));

    m_bossRing.setFillColor(sf::Color(0, 0, 0, 0));
    m_bossRing.setOutlineColor(Theme::NodeLine);
    m_bossRing.setOutlineThickness(2.f);

    m_label.setFont(font);                          // SFML 2.x API
    m_label.setCharacterSize(Theme::NodeTextSize);
    m_label.setFillColor(Theme::TextColor);
    m_label.setString(label);
    layoutLabel();

    m_bossText.setFont(font);
    m_bossText.setCharacterSize(14);
    m_bossText.setFillColor(Theme::NodeGlow);
    m_bossText.setOutlineColor(sf::Color(0, 0, 0, 220));
    m_bossText.setOutlineThickness(2.f);
    m_bossText.setString("BOSS");
    sf::FloatRect bb = m_bossText.getLocalBounds();
    m_bossText.setOrigin(bb.left + bb.width / 2.f, bb.top + bb.height / 2.f);
    m_bossText.setPosition(m_center.x, m_center.y - m_radius - 16.f);

    // Padlock: body rectangle + ring shackle above it.
    m_lockBody.setSize(sf::Vector2f(18.f, 14.f));
    m_lockBody.setOrigin(9.f, 7.f);
    m_lockBody.setPosition(m_center.x, m_center.y + 4.f);
    m_lockBody.setFillColor(Theme::NodeLockedLine);

    m_lockShackle.setRadius(6.f);
    m_lockShackle.setOrigin(6.f, 6.f);
    m_lockShackle.setPosition(m_center.x, m_center.y - 6.f);
    m_lockShackle.setFillColor(sf::Color(0, 0, 0, 0));
    m_lockShackle.setOutlineColor(Theme::NodeLockedLine);
    m_lockShackle.setOutlineThickness(3.f);

    buildStars();
    buildDashes();
    setState(0, false);
}

void LevelNode::layoutLabel() {
    sf::FloatRect b = m_label.getLocalBounds();
    m_label.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    m_label.setPosition(m_center);
}

void LevelNode::buildStars() {
    // 10-point star: alternate outer / inner radius.
    const float outer = Theme::StarRadius;
    const float inner = outer * 0.45f;
    const float gap = outer * 2.f + 4.f;
    const float y = m_center.y + m_radius + 18.f;

    for (int s = 0; s < 3; ++s) {
        m_star[s].setPointCount(10);
        for (int p = 0; p < 10; ++p) {
            float r = (p % 2 == 0) ? outer : inner;
            float a = -Theme::Pi / 2.f + p * (Theme::Pi / 5.f);
            m_star[s].setPoint(p, sf::Vector2f(r * std::cos(a), r * std::sin(a)));
        }
        m_star[s].setPosition(m_center.x + (s - 1) * gap, y);
        m_star[s].setOutlineThickness(1.5f);
    }
}

void LevelNode::buildDashes() {
    // Small rectangles around the circle, each rotated tangent to it.
    for (int i = 0; i < DashCount; ++i) {
        float a = (2.f * Theme::Pi * i) / DashCount;
        m_dash[i].setSize(sf::Vector2f(7.f, 3.f));
        m_dash[i].setOrigin(3.5f, 1.5f);
        m_dash[i].setFillColor(Theme::NodeLockedLine);
        m_dash[i].setPosition(m_center.x + (m_radius + 3.f) * std::cos(a),
            m_center.y + (m_radius + 3.f) * std::sin(a));
        m_dash[i].setRotation(a * 180.f / Theme::Pi + 90.f);
    }
}

void LevelNode::setState(int stars, bool unlocked) {
    m_unlocked = unlocked;
    m_stars = unlocked ? stars : 0;
    if (m_stars < 0) m_stars = 0;
    if (m_stars > 3) m_stars = 3;

    // Not colour alone: locked = padlock + dashes, done = stars.
    if (!m_unlocked) {
        m_body.setFillColor(Theme::NodeLocked);
        m_body.setOutlineColor(sf::Color(0, 0, 0, 0));   // dashes replace it
    }
    else if (m_stars > 0) {
        m_body.setFillColor(Theme::NodeDone);
        m_body.setOutlineColor(Theme::NodeLine);
    }
    else {
        m_body.setFillColor(Theme::NodeUnlocked);
        m_body.setOutlineColor(Theme::NodeLine);
    }

    for (int s = 0; s < 3; ++s) {
        bool on = s < m_stars;
        m_star[s].setFillColor(on ? Theme::StarOn : sf::Color(0, 0, 0, 0));
        m_star[s].setOutlineColor(on ? Theme::StarOn : Theme::StarOff);
    }
}

bool LevelNode::contains(int x, int y) const {
    float dx = static_cast<float>(x) - m_center.x;
    float dy = static_cast<float>(y) - m_center.y;
    return dx * dx + dy * dy <= m_radius * m_radius;
}

void LevelNode::handleEvent(const sf::Event& e) {
    // NOTE: raw window pixel coordinates (same as Button).
    if (e.type == sf::Event::MouseMoved) {
        if (m_state != StatePressed)
            m_state = contains(e.mouseMove.x, e.mouseMove.y) ? StateHover
            : StateIdle;
    }
    else if (e.type == sf::Event::MouseButtonPressed) {
        if (e.mouseButton.button == sf::Mouse::Left && m_unlocked &&
            contains(e.mouseButton.x, e.mouseButton.y))
            m_state = StatePressed;
    }
    else if (e.type == sf::Event::MouseButtonReleased) {
        if (e.mouseButton.button == sf::Mouse::Left &&
            m_state == StatePressed) {
            bool still = contains(e.mouseButton.x, e.mouseButton.y);
            m_state = still ? StateHover : StateIdle;
            // INTEGRATION: CampaignMap's callback will start the Briefing
            // for this level (act/level index passed through the context).
            if (still && m_unlocked && m_onClick) m_onClick(m_context);
        }
    }
}

void LevelNode::update(float dt) {
    m_time += dt;

    float target = (m_state == StateIdle) ? 0.f : 1.f;
    float step = 8.f * dt;
    if (m_hoverAmt < target) m_hoverAmt = minF(target, m_hoverAmt + step);
    else                     m_hoverAmt = maxF(target, m_hoverAmt - step);

    // Scale up on hover, shrink a little when pressed.
    float scale = 1.f + 0.12f * m_hoverAmt;
    if (m_state == StatePressed) scale = 0.95f;
    float r = m_radius * scale;

    m_body.setRadius(r);
    m_body.setOrigin(r, r);
    m_body.setPosition(m_center);

    // Pulsing glow ring. Locked nodes get a dim grey one (still feedback).
    float pulse = 4.f * std::sin(m_time * 5.f);
    float gr = r + 10.f + pulse;
    m_glow.setRadius(gr);
    m_glow.setOrigin(gr, gr);
    m_glow.setPosition(m_center);
    sf::Color gc = m_unlocked ? Theme::NodeGlow : Theme::NodeLockedLine;
    gc.a = static_cast<sf::Uint8>((m_unlocked ? 200.f : 90.f) * m_hoverAmt);
    m_glow.setOutlineColor(gc);
    m_glow.setOutlineThickness(4.f);

    // Not colour alone: outline also gets thicker on hover.
    if (m_unlocked) m_body.setOutlineThickness(3.f + 2.f * m_hoverAmt);

    if (m_boss) {
        float br = r + 7.f;
        m_bossRing.setRadius(br);
        m_bossRing.setOrigin(br, br);
        m_bossRing.setPosition(m_center);
    }

    // Label brightens on hover.
    m_label.setFillColor(lerpColor(Theme::TextColor, Theme::NodeGlow, m_hoverAmt));
}

void LevelNode::draw(sf::RenderWindow& window) const {
    if (m_hoverAmt > 0.01f) window.draw(m_glow);

    window.draw(m_body);
    if (m_boss) window.draw(m_bossRing);

    if (!m_unlocked) {
        for (int i = 0; i < DashCount; ++i) window.draw(m_dash[i]);
        window.draw(m_lockShackle);
        window.draw(m_lockBody);
    }
    else {
        window.draw(m_label);
        // Star slots only once the level has been completed.
        if (m_stars > 0)
            for (int s = 0; s < 3; ++s) window.draw(m_star[s]);
    }

    if (m_boss) window.draw(m_bossText);
}