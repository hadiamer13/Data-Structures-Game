// gui/screens/PlaceholderScreen.cpp
#include "PlaceholderScreen.h"
#include "../Theme.h"

namespace {
    // Centre a text object on a point (own helper, no <algorithm>).
    void centerOn(sf::Text& t, float x, float y) {
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
        t.setPosition(x, y);
    }
}  // namespace

PlaceholderScreen::PlaceholderScreen(AppContext& ctx, const char* title)
    : m_ctx(ctx) {
    const sf::Font& font = ctx.assets->font();

    // ---- Background -------------------------------------------------
    // TODO(YOU): reuses the menu background for now. When you have a
    // dedicated image for a screen (e.g. campaign map), add it in
    // Theme.h + Assets.cpp and swap the texture here.
    const sf::Texture& bg = ctx.assets->menuBackground();
    m_background.setTexture(bg);
    m_background.setScale(
        static_cast<float>(Theme::WindowW) / static_cast<float>(bg.getSize().x),
        static_cast<float>(Theme::WindowH) / static_cast<float>(bg.getSize().y));

    m_dim.setSize(sf::Vector2f(static_cast<float>(Theme::WindowW),
        static_cast<float>(Theme::WindowH)));
    m_dim.setFillColor(sf::Color(0, 0, 0, 150));

    m_title.setFont(font);
    m_title.setString(title);
    m_title.setCharacterSize(Theme::TitleSize);
    m_title.setFillColor(Theme::TitleColor);
    m_title.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_title.setOutlineThickness(3.f);
    centerOn(m_title, Theme::WindowW / 2.f, Theme::WindowH / 2.f - 40.f);

    m_hint.setFont(font);
    m_hint.setString("Coming soon - press Esc to return to the menu");
    m_hint.setCharacterSize(Theme::ButtonTextSize);
    m_hint.setFillColor(Theme::TextColor);
    centerOn(m_hint, Theme::WindowW / 2.f, Theme::WindowH / 2.f + 40.f);
}

void PlaceholderScreen::handleEvent(const sf::Event& e) {
    // Esc is handled globally by UIManager. Nothing to do here.
    // INTEGRATION (later): the real screen replaces this class entirely.
    (void)e;
}

void PlaceholderScreen::update(float dt) {
    (void)dt;
}

void PlaceholderScreen::draw(sf::RenderWindow& window) {
    window.draw(m_background);
    window.draw(m_dim);
    window.draw(m_title);
    window.draw(m_hint);
}