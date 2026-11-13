// gui/Theme.h
// Header-only constants. Change look & feel here only.
#pragma once
#include <SFML/Graphics.hpp>

namespace Theme {

    // ---- Window ----
    // TODO(YOU): set this to your background image size (e.g. 1280x720)
    inline const unsigned WindowW = 1280;
    inline const unsigned WindowH = 720;
    inline const unsigned FrameLimit = 60;

    // ---- Asset paths (relative to project root; run the exe from there) ----
    // TODO(YOU): put your font file here
    inline const char* const FontPath = "assets/fonts/vault.ttf";
    // TODO(YOU): put your menu background image here
    inline const char* const MenuBgPath = "assets/images/menu_bg.png";
    // TODO(YOU): put your campaign background image here
    inline const char* const CampaignBgPath = "assets/images/campaign_bg.png";
    // TODO(YOU): put your banner frame and skull icon here
    inline const char* const BannerPath = "assets/images/ui/banner.png";
    inline const char* const SkullPath = "assets/images/ui/icon_skull.png";

    // ---- Fonts ----
    inline const unsigned TitleSize = 64;
    inline const unsigned ButtonTextSize = 26;
    inline const unsigned NodeTextSize = 22;
    inline const unsigned TooltipTextSize = 18;

    // ---- Buttons ----
    inline const float ButtonW = 320.f;
    inline const float ButtonH = 56.f;
    inline const float ButtonGap = 18.f;

    // ---- Misc numbers ----
    inline const float Pi = 3.14159265f;
    inline const float NodeRadius = 30.f;     // old LevelNode (delete with it later)
    inline const float StarRadius = 9.f;
    inline const float BannerH = 340.f;       // selected banner height

    // ---- Colours (dark red theme) ----
    inline const sf::Color BtnIdle(28, 12, 16, 220);
    inline const sf::Color BtnHover(110, 20, 30, 235);
    inline const sf::Color BtnPressed(60, 10, 16, 245);
    inline const sf::Color BtnOutline(190, 40, 50);
    inline const sf::Color TextColor(235, 225, 220);
    inline const sf::Color TitleColor(220, 50, 55);

    inline const sf::Color NodeLocked(45, 40, 45, 230);
    inline const sf::Color NodeLockedLine(120, 110, 115);
    inline const sf::Color NodeUnlocked(50, 14, 20, 235);
    inline const sf::Color NodeDone(95, 20, 30, 240);
    inline const sf::Color NodeLine(190, 40, 50);
    inline const sf::Color NodeGlow(255, 90, 80);
    inline const sf::Color StarOn(255, 205, 80);
    inline const sf::Color StarOff(90, 70, 75);
    inline const sf::Color TooltipBg(14, 8, 12, 238);

    inline const sf::Color BannerSelected(255, 70, 70);
    inline const sf::Color BannerNormal(170, 55, 60, 215);
    inline const sf::Color BannerLocked(105, 100, 108, 190);
    inline const sf::Color BannerHover(255, 120, 105);

    // One colour per act (Briefing panel border uses these)
    inline const sf::Color ActColors[4] = {
        sf::Color(170, 40, 50, 200),
        sf::Color(150, 35, 60, 200),
        sf::Color(130, 30, 70, 200),
        sf::Color(190, 50, 40, 200)
    };

}  // namespace Theme