// gui/UIManager.h
#pragma once
#include "AppContext.h"
#include "screens/Screen.h"


// Screen IDs as plain ints (no enums).
namespace ScreenId {
    inline const int MainMenu = 0;
    inline const int Campaign = 1;
    inline const int Endless = 2;
    inline const int Sandbox = 3;
    inline const int Leaderboard = 4;
    inline const int Settings = 5;
    inline const int Briefing = 6;
    inline const int Level = 7;
    inline const int None = -1;
}  // namespace ScreenId

// Owns the current screen (raw pointer, deleted on switch / destructor).
class UIManager {
public:
    explicit UIManager(AppContext& ctx);
    ~UIManager();

    void start();                       // opens the main menu

    // Screens call these from callbacks. The switch is DEFERRED to the
    // start of the next update(), so a screen is never deleted while its
    // own handleEvent() is still running.
    void requestScreen(int id);
    void requestQuit();
    bool wantsQuit() const { return m_quit; }

    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    UIManager(const UIManager&);              // not copyable
    UIManager& operator=(const UIManager&);

    void    applyPending();
    Screen* makeScreen(int id);

    AppContext& m_ctx;
    Screen* m_current = nullptr;
    int         m_currentId = ScreenId::None;
    int         m_pendingId = ScreenId::None;
    bool        m_quit = false;
};