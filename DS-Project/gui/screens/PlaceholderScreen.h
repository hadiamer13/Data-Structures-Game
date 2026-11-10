// gui/screens/PlaceholderScreen.h
#pragma once
#include "Screen.h"
#include "../AppContext.h"

// "Coming soon" screen so every menu button has somewhere to go.
// Delete this class once all real screens exist.
class PlaceholderScreen : public Screen {
public:
    // title is a plain C string, copied into sf::Text.
    PlaceholderScreen(AppContext& ctx, const char* title);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    AppContext& m_ctx;
    sf::Sprite         m_background;
    sf::RectangleShape m_dim;       // dark overlay so text stays readable
    sf::Text           m_title;
    sf::Text           m_hint;
};