// gui/screens/ResultScreen.h
// Shows ctx.result (LevelResult): stars, steps vs optimal, penalties, score, sorted array.
// Retry -> Level, Next -> next Briefing (after 4.5: Main Menu), Campaign.
#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include "Screen.h"
#include "../AppContext.h"
#include "../widgets/Button.h"

class ResultScreen : public Screen
{
public:
    explicit ResultScreen(AppContext& ctx);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    // Called by the static callbacks (public so they can reach them).
    void onRetry();
    void onNext();
    void onCampaign();

private:
    static void onRetryClick(void* context);
    static void onNextClick(void* context);
    static void onCampaignClick(void* context);

    AppContext& m_ctx;
    bool m_won = false;
    bool m_lastLevel = false;
    bool m_showBadge = false;
    float m_fade = 0.f;

    sf::Sprite m_background;
    sf::RectangleShape m_dim;
    sf::RectangleShape m_panel;
    sf::RectangleShape m_badgeBox;
    sf::ConvexShape m_star[3];
    sf::Text m_title;
    sf::Text m_sub;
    sf::Text m_stats;
    sf::Text m_sorted;
    sf::Text m_badge;

    Button m_retry;
    Button m_next;
    Button m_campaign;
};