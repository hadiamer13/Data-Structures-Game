#pragma once

#include "Screen.h"
#include "../AppContext.h"
#include "../widgets/Button.h"
#include "../../game/Leaderboard.h"

//  report screen
// Campaign ke stars ka bar chart, aur endless ke top scores ka chart.
class ReportScreen : public Screen
{
public:
    explicit ReportScreen(AppContext& ctx);
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    static void onLeaderboard(void* context);
    static void onHome(void* context);
    void refreshCharts();
    AppContext& m_ctx;
    sf::Sprite m_background;
    sf::RectangleShape m_dim;
    sf::RectangleShape m_panel;
    sf::Text m_title;
    sf::Text m_campaignHeading;
    sf::Text m_scoreHeading;
    sf::Text m_campaignSummary;
    sf::Text m_emptyScores;
    sf::RectangleShape m_actBars[4];
    sf::Text m_actLabels[4];
    sf::RectangleShape m_scoreBars[Leaderboard::Capacity];
    sf::Text m_scoreLabels[Leaderboard::Capacity];
    Button m_leaderboard;
    Button m_home;
    Leaderboard m_scores;
};