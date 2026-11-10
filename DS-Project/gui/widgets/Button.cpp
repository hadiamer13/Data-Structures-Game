// gui/widgets/Button.cpp
#include "Button.h"
#include "../Theme.h"

namespace {

    // Own min/max (no <algorithm>).
    float minF(float a, float b) { return a < b ? a : b; }
    float maxF(float a, float b) { return a > b ? a : b; }

    // Linear blend of one colour channel.
    sf::Uint8 mixChannel(sf::Uint8 x, sf::Uint8 y, float t) {
        return static_cast<sf::Uint8>(x + (y - x) * t);
    }

    // Linear blend between two colours, t in [0,1].
    sf::Color lerpColor(const sf::Color& a, const sf::Color& b, float t) {
        return sf::Color(mixChannel(a.r, b.r, t), mixChannel(a.g, b.g, t),
            mixChannel(a.b, b.b, t), mixChannel(a.a, b.a, t));
    }

}  // namespace

void Button::init(const char* label, const sf::Font& font,
    sf::Vector2f center, sf::Vector2f size) {
    m_box.setSize(size);
    m_box.setOrigin(size.x / 2.f, size.y / 2.f);
    m_box.setPosition(center);
    m_box.setFillColor(Theme::BtnIdle);
    m_box.setOutlineColor(Theme::BtnOutline);
    m_box.setOutlineThickness(2.f);

    m_text.setFont(font);                          // SFML 2.x API
    m_text.setCharacterSize(Theme::ButtonTextSize);
    m_text.setFillColor(Theme::TextColor);
    setLabel(label);
}

void Button::setLabel(const char* label) {
    m_text.setString(label);
    centerText();
}

void Button::centerText() {
    sf::FloatRect b = m_text.getLocalBounds();
    m_text.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    m_text.setPosition(m_box.getPosition());
}

void Button::handleEvent(const sf::Event& e) {
    // NOTE: uses raw window pixel coordinates. If you later apply an
    // sf::View to the menu, convert with window.mapPixelToCoords first.
    sf::FloatRect bounds = m_box.getGlobalBounds();

    if (e.type == sf::Event::MouseMoved) {
        if (m_state != StatePressed) {
            bool in = bounds.contains(static_cast<float>(e.mouseMove.x),
                static_cast<float>(e.mouseMove.y));
            m_state = in ? StateHover : StateIdle;
        }
    }
    else if (e.type == sf::Event::MouseButtonPressed) {
        if (e.mouseButton.button == sf::Mouse::Left &&
            bounds.contains(static_cast<float>(e.mouseButton.x),
                static_cast<float>(e.mouseButton.y))) {
            m_state = StatePressed;
        }
    }
    else if (e.type == sf::Event::MouseButtonReleased) {
        if (e.mouseButton.button == sf::Mouse::Left &&
            m_state == StatePressed) {
            bool still = bounds.contains(static_cast<float>(e.mouseButton.x),
                static_cast<float>(e.mouseButton.y));
            m_state = still ? StateHover : StateIdle;
            // Fire only if released over the button.
            // INTEGRATION: the owner's callback decides what happens
            // (switch screen, quit, later: start a game session).
            if (still && m_onClick) m_onClick(m_context);
        }
    }
}

void Button::update(float dt) {
    // Ease hover amount toward target (~0.12 s fade).
    float target = (m_state == StateIdle) ? 0.f : 1.f;
    float step = 8.f * dt;
    if (m_hoverAmt < target) m_hoverAmt = minF(target, m_hoverAmt + step);
    else                     m_hoverAmt = maxF(target, m_hoverAmt - step);

    sf::Color fill = lerpColor(Theme::BtnIdle, Theme::BtnHover, m_hoverAmt);
    if (m_state == StatePressed) fill = Theme::BtnPressed;
    m_box.setFillColor(fill);

    // Not colour alone: outline gets thicker on hover/press.
    m_box.setOutlineThickness(2.f + 2.f * m_hoverAmt);
}

void Button::draw(sf::RenderWindow& window) const {
    window.draw(m_box);
    window.draw(m_text);
}