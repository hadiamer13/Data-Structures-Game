// gui/screens/Screen.h
#pragma once
#include <SFML/Graphics.hpp>

// Base class for every screen (menu, campaign map, level, ...).
class Screen {
public:
    virtual ~Screen() {}

    virtual void handleEvent(const sf::Event& e) = 0;
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
};