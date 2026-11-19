// gui/AppContext.h
// Passed to every screen so they can reach shared things without globals.
#pragma once
#include <SFML/Graphics.hpp>
#include "Assets.h"
#include "../contracts/Containers.h"

class UIManager;
class GameSession;
class Campaign;

struct GameSettings
{
    unsigned int antialiasingLevel;
    float animationSpeed;
    bool recreateWindow;

    GameSettings() : antialiasingLevel(4), animationSpeed(1.f), recreateWindow(false)
    {
    }
};

// Filled by the Level screen when a level ends, read by the Result screen.
struct LevelResult
{
    int stars;
    int alarm;
    int optimal;
    int penalties;
    int score;
    bool won;
    int levelIndex;        // act * 5 + level (0..19)
    IntArray sortedArray;  // Act 4: final array (empty otherwise)
    bool heapAct;

    LevelResult()
        : stars(0), alarm(0), optimal(0), penalties(0), score(0), won(false),
        levelIndex(0), heapAct(false)
    {
    }
};

struct AppContext
{
    sf::RenderWindow* window = nullptr;
    const Assets* assets = nullptr;
    UIManager* ui = nullptr;           // main_gui.cpp sets this to &uiManager
    GameSession* session = nullptr;    // main_gui.cpp sets this (stub or real)
    Campaign* campaign = nullptr;      // main_gui.cpp sets this; levels + star progress
    GameSettings* settings = nullptr;  // shared display and animation settings

    int selectedLevel = -1;            // index = act * 5 + level (0..19), -1 = none
    LevelResult result;
};
