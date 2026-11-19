// gui/UIManager.cpp
#include "UIManager.h"
#include "screens/MainMenu.h"
#include "screens/PlaceholderScreen.h"
#include "screens/CampaignMap.h"
#include "screens/BriefingScreen.h"
#include "screens/LevelScreen.h"
#include "screens/HeapLevel.h"
#include "screens/SandboxScreen.h"
#include "screens/ResultScreen.h"
#include "screens/EndlessScreen.h"
#include "screens/LeaderboardScreen.h"
#include "screens/SettingsScreen.h"
#include "screens/ReportScreen.h"

UIManager::UIManager(AppContext& ctx) : m_ctx(ctx)
{
}

UIManager::~UIManager()
{
    delete m_current;
    m_current = nullptr;
}

void UIManager::start()
{
    m_pendingId = ScreenId::MainMenu;
    applyPending();
}

void UIManager::requestScreen(int id)
{
    m_pendingId = id;
}

void UIManager::requestQuit()
{
    m_quit = true;
}

Screen* UIManager::makeScreen(int id)
{
    // ---- INTEGRATION POINT -------------------------------------------
    // The only place that maps a screen ID to a screen object.
    // ------------------------------------------------------------------
    switch (id)
    {
    case ScreenId::MainMenu:
        return new MainMenu(m_ctx);
    case ScreenId::Campaign:
        return new CampaignMap(m_ctx);
    case ScreenId::Endless:
        return new EndlessScreen(m_ctx);
    case ScreenId::Sandbox:
        return new SandboxScreen(m_ctx);
    case ScreenId::Leaderboard:
        return new LeaderboardScreen(m_ctx);
    case ScreenId::Settings:
        return new SettingsScreen(m_ctx);
    case ScreenId::Reports:
        return new ReportScreen(m_ctx);
    case ScreenId::Briefing:
        return new BriefingScreen(m_ctx);
    case ScreenId::Level:
        return new LevelScreen(m_ctx);
    case ScreenId::HeapLevel:
        return new HeapLevel(m_ctx);
    case ScreenId::Result:
        return new ResultScreen(m_ctx);
    default:
        return nullptr;
    }
}

void UIManager::applyPending()
{
    if (m_pendingId == ScreenId::None)
    {
        return;
    }

    Screen* next = makeScreen(m_pendingId);
    if (next != nullptr)
    {
        delete m_current;          // safe: we are not inside its handler
        m_current = next;
        m_currentId = m_pendingId;
    }
    m_pendingId = ScreenId::None;
}

void UIManager::handleEvent(const sf::Event& e)
{
    // Global Esc: Briefing, Level, HeapLevel and Result step back to the Campaign Map,
    // every other screen returns to the Main Menu.
    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape)
    {
        if (m_currentId == ScreenId::Briefing || m_currentId == ScreenId::Level ||
            m_currentId == ScreenId::HeapLevel || m_currentId == ScreenId::Result)
        {
            requestScreen(ScreenId::Campaign);
            return;
        }
        if (m_currentId != ScreenId::MainMenu)
        {
            requestScreen(ScreenId::MainMenu);
            return;
        }
    }
    if (m_current)
    {
        m_current->handleEvent(e);
    }
}

void UIManager::update(float dt)
{
    applyPending();                // switches happen here, once per frame
    if (m_current)
    {
        m_current->update(dt);
    }
}

void UIManager::draw(sf::RenderWindow& window)
{
    if (m_current)
    {
        m_current->draw(window);
    }
}
