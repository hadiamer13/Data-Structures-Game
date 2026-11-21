#pragma once
#include "Screen.h"
#include "../AppContext.h"
#include "../widgets/Button.h"
#include "../widgets/LevelBanner.h"
#include "../../game/Campaign.h"

//  campaign map
// Char acts ka map. Har act mein paanch level banners hain, aur neeche selected level ki info.
// Four acts shown as a map. Each act has five level banners and an info panel for the selected level.
class CampaignMap : public Screen
{
  public:
    explicit CampaignMap(AppContext& ctx);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    // Static callbacks se call hote hain, isliye public hain.
    // Public because the static callbacks call them.
    void onBannerClicked(int slot);
    void onStart();
    void onBack();
    void onDebugUnlock();

    // Stars aur locks ko Campaign se dobara parhta hai, jaise result save hone ke baad.
    // Re-reads stars and locks from the Campaign, for example after a result is saved.
    void refreshFromData();

  private:
    static const int ActCount = 4;
    static const int PerAct = 5;

    struct BannerCtx
    {
        CampaignMap* map;
        int slot;     // act ke andar 0 se 4 tak
    };

    static void onBannerClick(void* context);
    static void onStartClick(void* context);
    static void onBackClick(void* context);
    static void onDebugClick(void* context);

    int levelIndex() const
    {
        return m_act * PerAct + m_sel;
    }

    bool startEnabled() const;
    void setAct(int act);
    void setSelected(int slot);
    void initBanners();
    void refreshInfo();
    void loadBannerStates();
    void refreshDebugLabel();
    void buildArrow(sf::ConvexShape& s, bool left, float cx, float cy);
    void buildStars(float x, float y);
    void styleText(sf::Text& t, const sf::Font& font, unsigned size,
                   const sf::Color& col, float x, float y);

    AppContext& m_ctx;
    Campaign* m_campaign;
    const sf::Font* m_font;

    int m_act = 0;      // 0 se 3 tak
    int m_sel = 0;      // is act mein selected banner, 0 se 4 tak

    //  backdrop and decoration
    sf::Sprite         m_background;
    sf::RectangleShape m_dim;
    sf::Sprite         m_corners[4];   // TL, TR, BL, BR
    sf::Sprite         m_divider;      // act ke naam ke neeche star ornament

    //  top bar
    sf::Text           m_tab[ActCount];
    sf::Sprite         m_tabHighlight;  // selected act ke neeche T-shaped marker
    sf::FloatRect      m_tabRect[ActCount];
    int                m_tabHover = -1;
    sf::ConvexShape    m_arrow[2];      // 0 left, 1 right
    sf::FloatRect      m_arrowRect[2];
    float              m_arrowAmt[2];   // hover ka smooth 0 se 1 value
    bool               m_arrowHover[2];
    sf::Text           m_actName;

    //  banners
    LevelBanner m_banner[PerAct];
    BannerCtx m_bannerCtx[PerAct];

    //  info panel
    sf::RectangleShape m_panel;
    sf::Text m_title, m_status, m_concept, m_objective;
    sf::Text m_bestLabel, m_lockedMsg;
    sf::ConvexShape m_star[3];
    float m_panelX = 0.f, m_panelY = 0.f, m_panelW = 0.f, m_panelH = 0.f;

    Button m_start;
    Button m_back;
    Button m_debug;     // DEBUG: sab levels unlock karne ka toggle
};