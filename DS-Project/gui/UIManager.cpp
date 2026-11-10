// gui/UIManager.cpp
#include "UIManager.h"
#include "screens/MainMenu.h"
#include "screens/PlaceholderScreen.h"   
#include "screens/CampaignMap.h"

UIManager::UIManager(AppContext& ctx) : m_ctx(ctx) {}

UIManager::~UIManager() {
    delete m_current;
    m_current = nullptr;
}

void UIManager::start() {
    m_pendingId = ScreenId::MainMenu;
    applyPending();
}

void UIManager::requestScreen(int id) { m_pendingId = id; }
void UIManager::requestQuit() { m_quit = true; }

Screen* UIManager::makeScreen(int id) {
    // ---- INTEGRATION POINT -------------------------------------------
    // This is the ONLY place to edit when a real screen replaces a
    // placeholder. Example later:
    //   case ScreenId::Campaign: return new CampaignMap(m_ctx);
    // ------------------------------------------------------------------
    switch (id) {
    case ScreenId::MainMenu:    return new MainMenu(m_ctx);
    case ScreenId::Campaign:    return new CampaignMap(m_ctx);
    case ScreenId::Endless:     return new PlaceholderScreen(m_ctx, "Endless Mode");
    case ScreenId::Sandbox:     return new PlaceholderScreen(m_ctx, "Sandbox");
    case ScreenId::Leaderboard: return new PlaceholderScreen(m_ctx, "Leaderboard");
    case ScreenId::Settings:    return new PlaceholderScreen(m_ctx, "Settings");
    default:                    return nullptr;
    }
}

void UIManager::applyPending() {
    if (m_pendingId == ScreenId::None) return;

    Screen* next = makeScreen(m_pendingId);
    if (next != nullptr) {
        delete m_current;          // safe: we are not inside its handler
        m_current = next;
        m_currentId = m_pendingId;
    }
    m_pendingId = ScreenId::None;
}

void UIManager::handleEvent(const sf::Event& e) {
    // Global key: Esc returns to the menu from any other screen.
    // (Later, Level screens may want Esc to open a pause dialog instead.)
    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) {
        if (m_currentId != ScreenId::MainMenu) {
            requestScreen(ScreenId::MainMenu);
            return;
        }
    }
    if (m_current) m_current->handleEvent(e);
}

void UIManager::update(float dt) {
    applyPending();                // switches happen here, once per frame
    if (m_current) m_current->update(dt);
}

void UIManager::draw(sf::RenderWindow& window) {
    if (m_current) m_current->draw(window);
}