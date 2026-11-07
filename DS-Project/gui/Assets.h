#pragma once
#include <SFML/Graphics/Font.hpp>

class Assets {
public:
    bool load();                              // call once at startup
    const sf::Font& font() const { return m_font; }
private:
    sf::Font m_font;
};