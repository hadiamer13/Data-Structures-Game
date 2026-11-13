// gui/screens/CampaignMap.h
#pragma once
#include "Screen.h"
#include "../AppContext.h"
#include "../widgets/Button.h"
#include "../widgets/LevelBanner.h"

class CampaignMap : public Screen {
public:
    explicit CampaignMap(AppContext& ctx);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    // Called by the static callbacks (public so they can reach it).
    void onBannerClicked(int slot);
    void onStart();
    void onBack();

    // INTEGRATION (later): call after SaveManager loads or a level ends,
    // so stars/locks update without rebuilding the screen.
    void refreshFromData();

private:
    static const int ActCount = 4;
    static const int PerAct = 5;

    struct BannerCtx {
        CampaignMap* map;
        int          slot;     // 0..4 inside the current act
    };
    static void onBannerClick(void* context);
    static void onStartClick(void* context);
    static void onBackClick(void* context);

    int  levelIndex() const { return m_act * PerAct + m_sel; }
    void setAct(int act);
    void setSelected(int slot);
    void refreshInfo();
    void loadBannerStates();
    void buildArrow(sf::ConvexShape& s, bool left, float cx, float cy);
    void buildStars(float x, float y);
    void styleText(sf::Text& t, const sf::Font& font, unsigned size,
        const sf::Color& col, float x, float y);

    AppContext& m_ctx;
    int m_act = 0;      // 0..3
    int m_sel = 0;      // 0..4 (selected banner in this act)

    // Backdrop and decoration
    sf::Sprite         m_background;
    sf::RectangleShape m_dim;
    sf::RectangleShape m_corner[8];     // 4 L-shaped corners, 2 bars each
    sf::RectangleShape m_divider;
    sf::ConvexShape    m_diamond;

    // Top bar
    sf::Text           m_tab[ActCount];
    sf::RectangleShape m_tabUnder;
    sf::FloatRect      m_tabRect[ActCount];
    int                m_tabHover = -1;
    sf::ConvexShape    m_arrow[2];      // 0 = left, 1 = right
    sf::FloatRect      m_arrowRect[2];
    float              m_arrowAmt[2];   // eased hover 0..1
    bool               m_arrowHover[2];
    sf::Text           m_actName;

    // Banners
    LevelBanner m_banner[PerAct];
    BannerCtx   m_bannerCtx[PerAct];

    // Info panel
    sf::RectangleShape m_panel;
    sf::Text m_title, m_status, m_concept, m_objective;
    sf::Text m_bestLabel, m_lockedMsg;
    sf::ConvexShape m_star[3];
    float m_panelX = 0.f, m_panelY = 0.f, m_panelW = 0.f, m_panelH = 0.f;

    Button m_start;
    Button m_back;
};