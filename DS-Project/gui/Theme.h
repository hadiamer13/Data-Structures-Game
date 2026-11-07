#pragma once
// gui/Theme.h - colours and sizes. SFML allowed here (gui only).
// Usage: Theme::balanced(), Theme::NodeRadius
#include <SFML/Graphics/Color.hpp>

struct Theme {
    // Colour code (never the only signal: always add a label or outline too)
    static sf::Color balanced() { return sf::Color(60, 170, 90); }    // green
    static sf::Color warning() { return sf::Color(230, 200, 60); }   // yellow
    static sf::Color violation() { return sf::Color(210, 60, 60); }    // red
    static sf::Color visit() { return sf::Color(70, 130, 230); }   // blue
    static sf::Color skipped() { return sf::Color(130, 130, 130); }  // grey
    static sf::Color target() { return sf::Color(240, 190, 40); }   // gold

    static sf::Color background() { return sf::Color(24, 20, 32); }
    static sf::Color panel() { return sf::Color(40, 34, 54); }
    static sf::Color text() { return sf::Color(235, 230, 220); }

    static inline const float    NodeRadius = 22.0f;
    static inline const float    OutlineThick = 3.0f;
    static inline const unsigned FontSize = 18;
};