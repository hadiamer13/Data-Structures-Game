// gui/widgets/Tooltip.h
// Chota hover box. Owner decide karta hai KAB dikhana hai aur KYA likhna hai;
// box sirf khud ko position karta (window ke andar clamp karke) aur draw karta hai.
#pragma once
#include <string>
#include <SFML/Graphics.hpp>

class Tooltip
{
  public:
    Tooltip();

    void init(const sf::Font& font);

    // Key, height, balance factor, size, depth. showBalance = AVL (bf aur status word bhi add hota hai).
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