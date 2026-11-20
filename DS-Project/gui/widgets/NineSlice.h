// gui/widgets/NineSlice.h
// Draws a texture as a frame: corners and edges keep their native size and only
// the middle stretches. Use it for every framed decoration so the art never
// distorts when a button or bar is wide.
#pragma once
#include <SFML/Graphics.hpp>

class NineSlice
{
public:
    NineSlice();

    // border = width of the untouched corner/edge band, in texture pixels.
    // Passing nullptr clears the frame.
    void setTexture(const sf::Texture* tex, int border);

    // The on-screen rectangle the whole frame fills.
    void setRect(const sf::FloatRect& outer);

    void setColor(const sf::Color& c);
    bool hasTexture() const;

    void draw(sf::RenderWindow& window) const;

private:
    void rebuild();

    sf::Sprite         m_part[9];   // row-major: 0..2 top, 3..5 middle, 6..8 bottom
    const sf::Texture* m_tex;
    int                m_border;
    sf::FloatRect      m_rect;
    sf::Color          m_color;
};