#pragma once
// gui/AppContext.h - shared pointers passed to every screen.
#include <SFML/Graphics/RenderWindow.hpp>
#include "gui/Assets.h"

struct AppContext {
    sf::RenderWindow* window = nullptr;
    Assets* assets = nullptr;
    // GameSession* session  = nullptr;   // added later
};