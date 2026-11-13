// gui/AppContext.h
// Passed to every screen so they can reach shared things without globals.
#pragma once
#include <SFML/Graphics.hpp>
#include "Assets.h"

class UIManager;  // forward declaration (avoids circular include)

struct AppContext {
    sf::RenderWindow* window = nullptr;
    const Assets* assets = nullptr;
    UIManager* ui = nullptr;   // main_gui.cpp sets this to &uiManager

    int selectedLevel = -1;
    // INTEGRATION (later): add `GameSession* session = nullptr;` here so
    // Level/Sandbox screens can call session->apply(...). Leave it out for now.
};