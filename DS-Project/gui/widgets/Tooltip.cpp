// gui/widgets/Tooltip.cpp
#include "Tooltip.h"
#include "../Theme.h"

namespace
{
    const float Pad = 8.f;
}

Tooltip::Tooltip() : m_visible(false)
{
}

void Tooltip::init(const sf::Font& font)
{
    m_text.setFont(font);
    m_text.setCharacterSize(Theme::TooltipTextSize);
    m_text.setFillColor(Theme::TextColor);

    m_box.setFillColor(Theme::TooltipBg);
    m_box.setOutlineColor(Theme::BtnOutline);
    m_box.setOutlineThickness(2.f);
}

// Text set karke box ka size aur position tay karta hai (window ke andar rehne ke liye clamp).
void Tooltip::showText(float sx, float sy, const std::string& text)
{
    m_text.setString(text);    // SFML string same ho to dobara rebuild nahi karta
    sf::FloatRect b = m_text.getLocalBounds();
    float w = b.left + b.width + 2.f * Pad;
    float h = b.top + b.height + 2.f * Pad;

    // Poora box window ke andar rahe.
    float maxX = (float)Theme::WindowW - w - 4.f;
    float maxY = (float)Theme::WindowH - h - 4.f;
    if (sx > maxX)
    {
        sx = maxX;
    }
    if (sy > maxY)
    {
        sy = maxY;
    }
    if (sx < 4.f)
    {
        sx = 4.f;
    }
    if (sy < 4.f)
    {
        sy = 4.f;
    }

    m_box.setSize(sf::Vector2f(w, h));
    m_box.setPosition(sx, sy);
    m_text.setPosition(sx + Pad, sy + Pad);
    m_visible = true;
}

// Node ki maloomat ko formatted text mein badal kar dikhata hai.
void Tooltip::showNode(float sx, float sy, int key, int height, int balance, int size, int depth,
    bool showBalance)
{
    std::string s = "key: " + std::to_string(key)
        + "\nheight: " + std::to_string(height);
    if (showBalance)
    {
        int ab = (balance < 0) ? -balance : balance;
        s += "\nbalance: ";
        if (balance > 0)
        {
            s += "+";
        }
        s += std::to_string(balance);
        s += (ab <= 1) ? "  (OK)" : "  (VIOLATION)";   // word, sirf rang nahi
    }
    s += "\nsize: " + std::to_string(size)
        + "\ndepth: " + std::to_string(depth);
    showText(sx, sy, s);
}

void Tooltip::hide()
{
    m_visible = false;
}

bool Tooltip::visible() const
{
    return m_visible;
}

void Tooltip::draw(sf::RenderWindow& window) const
{
    if (!m_visible)
    {
        return;
    }
    window.draw(m_box);
    window.draw(m_text);
}