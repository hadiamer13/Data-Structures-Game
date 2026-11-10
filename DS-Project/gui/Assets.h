// gui/Assets.h
#pragma once
#include <SFML/Graphics.hpp>

// Loads fonts/textures ONCE and hands out const references.
// Never load assets inside a screen's draw/update loop.
class Assets {
public:
    // Returns false if any required asset failed to load.
    bool loadAll();

    const sf::Font& font() const { return m_font; }
    const sf::Texture& menuBackground() const { return m_menuBg; }
    const sf::Texture& campaignBackground() const { return m_campaignBg; }


    // Later: add accessors for campaign map bg, node icons, vault door, etc.

private:
    sf::Font    m_font;
    sf::Texture m_menuBg;
    sf::Texture m_campaignBg;   
};