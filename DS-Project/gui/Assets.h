// gui/Assets.h
#pragma once
#include <SFML/Graphics.hpp>

// Loads fonts/textures ONCE and hands out const references.
// Never load assets inside a screen's draw/update loop.
class Assets
{
  public:
    // Returns false if any required asset failed to load.
    bool loadAll();

    const sf::Font& font() const
    {
        return m_font;
    }

    const sf::Texture& menuBackground() const
    {
        return m_menuBg;
    }

    const sf::Texture& campaignBackground() const
    {
        return m_campaignBg;
    }

    const sf::Texture& banner() const
    {
        return m_banner;
    }

    // UI sheet pieces
    const sf::Texture& skull() const
    {
        return m_skull;
    }

    const sf::Texture& nodeRing() const
    {
        return m_nodeRing;
    }

    const sf::Texture& buttonFrame() const
    {
        return m_buttonFrame;
    }

    const sf::Texture& divider() const
    {
        return m_divider;
    }

    const sf::Texture& dividerAlt() const
    {
        return m_dividerAlt;
    }

    const sf::Texture& highlight() const
    {
        return m_highlight;
    }

    const sf::Texture& corner() const
    {
        return m_corner;
    }

    const sf::Texture& cornerSmall() const
    {
        return m_cornerSmall;
    }

  private:
    sf::Font m_font;
    sf::Texture m_menuBg;
    sf::Texture m_campaignBg;
    sf::Texture m_banner;

    sf::Texture m_skull;
    sf::Texture m_nodeRing;
    sf::Texture m_buttonFrame;
    sf::Texture m_divider;
    sf::Texture m_dividerAlt;
    sf::Texture m_highlight;
    sf::Texture m_corner;
    sf::Texture m_cornerSmall;
};