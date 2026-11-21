// gui/AppContext.h
// Yeh struct har screen ko milta hai taake woh shared cheezon tak pohonch sake
// (global variables ke bajaye).
#pragma once
#include <SFML/Graphics.hpp>
#include "Assets.h"
#include "../contracts/Containers.h"

class UIManager;
class GameSession;
class Campaign;

// Game ki display aur animation settings. Settings screen inhe change karti hai.
struct GameSettings
{
    unsigned int antialiasingLevel;
    float animationSpeed;
    bool recreateWindow;

    GameSettings() : antialiasingLevel(4), animationSpeed(1.f), recreateWindow(false)
    {
    }
};

// Level khatam hone par Level screen yeh data bharti hai, aur Result screen ise parhti hai.
struct LevelResult
{
    int stars;
    int alarm;
    int optimal;
    int penalties;
    int score;
    bool won;
    int levelIndex;        // act * 5 + level (0..19)
    IntArray sortedArray;  // Act 4 ka final sorted array (warna khali)
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
    UIManager* ui = nullptr;           // main_gui.cpp isay &uiManager par set karta hai
    GameSession* session = nullptr;    // main_gui.cpp isay set karta hai (stub ya asli session)
    Campaign* campaign = nullptr;      // main_gui.cpp isay set karta hai; levels aur star progress
    GameSettings* settings = nullptr;  // display aur animation ki shared settings

    int selectedLevel = -1;            // index = act * 5 + level (0..19), -1 ka matlab koi nahi
    LevelResult result;
};