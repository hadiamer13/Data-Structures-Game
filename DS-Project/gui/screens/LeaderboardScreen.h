#pragma once

#include "Screen.h"
#include "../AppContext.h"
#include "../widgets/Button.h"
#include "../../game/Leaderboard.h"

class LeaderboardScreen : public Screen
{
public:
    explicit LeaderboardScreen(AppContext& ctx);
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    static void onHome(void* context);
    static void onReports(void* context);
    AppContext& m_ctx;
    sf::Sprite m_background;
    sf::RectangleShape m_dim;
    sf::RectangleShape m_panel;
    sf::Text m_title;
    sf::Text m_rows[Leaderboard::Capacity];
    sf::Text m_empty;
    sf::Text m_error;
    Button m_home;
    Button m_reports;
    Leaderboard m_scores;
    bool m_loaded;
};
