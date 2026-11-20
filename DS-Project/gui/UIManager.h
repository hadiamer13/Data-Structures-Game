// gui/UIManager.h
#pragma once
#include "AppContext.h"
#include "screens/Screen.h"

// Screen IDs plain int ki tarah hain (enum nahi).
namespace ScreenId {
    inline const int MainMenu = 0;
    inline const int Campaign = 1;
    inline const int Endless = 2;
    inline const int Sandbox = 3;
    inline const int Leaderboard = 4;
    inline const int Settings = 5;
    inline const int Briefing = 6;
    inline const int Level = 7;
    inline const int Result = 8;
    inline const int HeapLevel = 9;   // Act 4 (campaign index 15..19)
    inline const int Reports = 10;
    inline const int None = -1;
}  

// Current screen ka owner (raw pointer, switch ya destructor par delete hota hai).
class UIManager {
public:
    explicit UIManager(AppContext& ctx);
    ~UIManager();

    void start();        // main menu kholta hai

    // Screens callbacks se yeh call karte hain. Switch DEFERRED hai,
    // agle update() ke shuru tak. Is se screen apne handleEvent() ke chalte hue delete nahi hoti.
    void requestScreen(int id);
    void requestQuit();
    bool wantsQuit() const { return m_quit; }

    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    UIManager(const UIManager&);             
    UIManager& operator=(const UIManager&);

    void    applyPending();
    Screen* makeScreen(int id);

    AppContext& m_ctx;
    Screen* m_current = nullptr;
    int m_currentId = ScreenId::None;
    int m_pendingId = ScreenId::None;
    bool m_quit = false;
};