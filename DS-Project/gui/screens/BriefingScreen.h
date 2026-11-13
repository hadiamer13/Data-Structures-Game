// gui/screens/BriefingScreen.h
#pragma once
#include "Screen.h"
#include "../AppContext.h"
#include "../widgets/Button.h"

class BriefingScreen : public Screen {
public:
    explicit BriefingScreen(AppContext& ctx);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    // Called by the static callbacks (public so they can reach it).
    void onStart();
    void onBack();

private:
    static void onStartClick(void* context);
    static void onBackClick(void* context);

    void makeLabel(sf::Text& t, const sf::Font& font, const char* str,
        unsigned size, const sf::Color& col, float x, float y);
    void buildStars(float x, float y);

    AppContext& m_ctx;
    int         m_index = 0;       // level index 0..19
    int         m_stars = 0;       // best stars for this level

    sf::Sprite         m_background;
    sf::RectangleShape m_dim;
    sf::RectangleShape m_panel;
    sf::RectangleShape m_divider;

    sf::Text m_act, m_title, m_boss;
    sf::Text m_objHead, m_objective;
    sf::Text m_conceptHead, m_concept;
    sf::Text m_ruleHead, m_rule;
    sf::Text m_bestLabel;

    sf::ConvexShape m_star[3];

    Button m_start;
    Button m_backBtn;

    float m_fade = 0.f;            // 0..1 fade-in of the dim layer
};