// gui/widgets/Ornament.h
// Ornament sprites ko jagah dene ke helpers. Header-only, koi state nahi.
// Khali textures skip hoti hain, is liye missing file se divide by zero nahi hota.
#pragma once
#include <SFML/Graphics.hpp>

// Corner ornament ko iske top-left point (x, y) par rakhta hai.
// flipX / flipY se aik hi texture chaaron konon ke kaam aati hai.
inline void placeCorner(sf::Sprite& s, const sf::Texture& t,
                        float x, float y, float size, bool flipX, bool flipY)
{
    const sf::Vector2u ts = t.getSize();
    if (ts.x == 0 || ts.y == 0)
    {
        return;
    }

    const float maxDim = static_cast<float>(ts.x > ts.y ? ts.x : ts.y);
    const float k = size / maxDim;
    s.setTexture(t);
    s.setOrigin(flipX ? static_cast<float>(ts.x) : 0.f,
                flipY ? static_cast<float>(ts.y) : 0.f);
    s.setScale(flipX ? -k : k, flipY ? -k : k);
    s.setPosition(x, y);
}

// Horizontal divider ko (cx, cy) par center karta hai.
// Art ka aspect ratio qaim rehta hai, width ke hisaab se scale hota hai,
// lekin maxHeight se zyada lamba nahi hota taake upar neeche ka text na chhoye.
// maxHeight <= 0 ka matlab koi height limit nahi.
inline void placeDivider(sf::Sprite& s, const sf::Texture& t,
                         float cx, float cy, float width, float maxHeight = 0.f)
{
    const sf::Vector2u ts = t.getSize();
    if (ts.x == 0 || ts.y == 0)
    {
        return;
    }

    float k = width / static_cast<float>(ts.x);
    if (maxHeight > 0.f)
    {
        const float kH = maxHeight / static_cast<float>(ts.y);
        if (kH < k)
        {
            k = kH;
        }
    }
    s.setTexture(t);
    s.setOrigin(ts.x / 2.f, ts.y / 2.f);
    s.setScale(k, k);
    s.setPosition(cx, cy);
}