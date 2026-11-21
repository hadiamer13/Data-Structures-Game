#pragma once
#include <SFML/Graphics.hpp>

//  base screen
// Har screen ka base class. Har screen ko handleEvent, update aur draw dena zaroori hai.
class Screen {
public:
    virtual ~Screen() {}

    virtual void handleEvent(const sf::Event& e) = 0;
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
};