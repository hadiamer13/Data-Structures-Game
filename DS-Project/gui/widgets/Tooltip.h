// gui/widgets/Tooltip.h
// Small hover box. The owner decides WHEN to show it and WHAT it says;
// the box only positions itself (clamped inside the window) and draws.
#pragma once
#include <string>
#include <SFML/Graphics.hpp>

class Tooltip
{
public:
    Tooltip();

    void init(const sf::Font& font);

    // Key, height, balance factor, size, depth. showBalance = AVL (adds bf + status word).
    void showNode(float sx, float sy, int key, int height, int balance, int size, int depth,
        bool showBalance);
    void showText(float sx, float sy, const std::string& text);
    void hide();
    bool visible() const;

    void draw(sf::RenderWindow& window) const;

private:
    sf::RectangleShape m_box;
    sf::Text m_text;
    bool m_visible;
};
