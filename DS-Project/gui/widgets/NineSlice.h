// gui/widgets/NineSlice.h
// Texture ko frame ke taur par draw karta hai: kone aur kinare apna asli size rakhte hain
// aur sirf beech ka hissa stretch hota hai. Isse wide button ya bar par art distort nahi hota.
#pragma once
#include <SFML/Graphics.hpp>

class NineSlice
{
  public:
    NineSlice();

    // border = kone/kinare ka woh hissa jo nahi badalta, texture pixels mein.
    // nullptr dene par frame clear ho jata hai.
    void setTexture(const sf::Texture* tex, int border);

    // Poore frame ka screen par rectangle.
    void setRect(const sf::FloatRect& outer);

    void setColor(const sf::Color& c);
    bool hasTexture() const;

    void draw(sf::RenderWindow& window) const;

private:
    // Sprites ko dobara calculate karta hai (texture ya rect badalne par).
    void rebuild();

    sf::Sprite         m_part[9];   // row-major: 0..2 upar, 3..5 beech, 6..8 neeche
    const sf::Texture* m_tex;
    int m_border;
    sf::FloatRect m_rect;
    sf::Color m_color;
};