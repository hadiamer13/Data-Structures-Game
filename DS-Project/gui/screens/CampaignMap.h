// gui/screens/CampaignMap.h
#pragma once
#include "Screen.h"
#include "../AppContext.h"
#include "../widgets/Button.h"
#include "../widgets/LevelNode.h"

class CampaignMap : public Screen {
public:
    explicit CampaignMap(AppContext& ctx);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    // Called by the static callbacks (public so they can reach it).
    void onNodeChosen(int index);
    void onBack();

    // INTEGRATION (later): call this after SaveManager loads or a level
    // finishes, so stars/locks/links update without rebuilding the screen.
    void refreshFromData();

private:
    static const int ActCount = 4;
    static const int PerAct = 5;
    static const int LevelCount = 20;
    static const int LinkCount = 16;   // 4 links per act

    struct NodeCtx {
        CampaignMap* map;
        int          index;
    };
    static void onNodeClick(void* context);
    static void onBackClick(void* context);

    void layoutTooltip(int idx);

    AppContext& m_ctx;

    sf::Sprite         m_background;
    sf::RectangleShape m_dim;
    sf::RectangleShape m_banner[ActCount];
    sf::Text           m_actTitle[ActCount];
    sf::RectangleShape m_link[LinkCount];

    LevelNode m_nodes[LevelCount];
    NodeCtx   m_nodeCtx[LevelCount];
    Button    m_back;

    // Tooltip: shapes/texts created once, strings only set on change.
    int                m_hoverIdx = -1;
    sf::RectangleShape m_tipBox;
    sf::Text           m_tipTitle;
    sf::Text           m_tipConcept;
    sf::Text           m_tipStatus;

    // Toast shown after clicking a node (placeholder for the Briefing).
    sf::Text m_toast;
    float    m_toastTime = 0.f;
};