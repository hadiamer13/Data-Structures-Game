// gui/widgets/Ornament.h
// Placement helpers for ornament sprites. Header-only, no state.
// Empty textures are skipped, so a missing file never causes a divide by zero.
#pragma once
#include <SFML/Graphics.hpp>

// Places a corner ornament so its top-left point sits at (x, y).
// flipX / flipY mirror it, so one texture serves all four corners.
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

// Places a horizontal divider centred at (cx, cy).
// The art keeps its proportions. It is scaled to the requested width,
// but never taller than maxHeight, so tall ornament peaks can't reach
// the text above or below. maxHeight <= 0 means no height limit.
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