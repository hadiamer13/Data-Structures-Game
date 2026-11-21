// gui/Theme.h
// Sirf constants. Look aur feel badalna ho to bas yahin badlo.
#pragma once
#include <SFML/Graphics.hpp>

namespace Theme {

    // Window
    
    inline const unsigned WindowW = 1280;
    inline const unsigned WindowH = 720;
    inline const unsigned FrameLimit = 60;

    // Asset paths (project root se relative; exe ko project root se chalao)
    inline const char* const FontPath = "assets/fonts/vault.ttf";
    inline const char* const MenuBgPath = "assets/images/menu_bg.png";
    inline const char* const CampaignBgPath = "assets/images/campaign_bg.png";
    inline const char* const BannerPath = "assets/images/ui/banner.png";

    // UI sheet ke hisse (sprite_cutter.py se kaatay gaye)
    inline const char* const SkullPath = "assets/images/ui/ui_skull_01.png";
    inline const char* const NodeRingPath = "assets/images/ui/ui_ring_01.png";
    inline const char* const ButtonFramePath = "assets/images/ui/ui_title_06.png";
    inline const char* const DividerPath = "assets/images/ui/ui_divider_04.png";
    inline const char* const DividerAltPath = "assets/images/ui/ui_divider_08.png";
    inline const char* const HighlightPath = "assets/images/ui/ui_corner_07.png";
    inline const char* const CornerPath = "assets/images/ui/ui_corner_03.png";
    inline const char* const CornerSmallPath = "assets/images/ui/ui_small_08.png";

    // Font sizes
    inline const unsigned TitleSize = 48;
    inline const unsigned ButtonTextSize = 20;
    inline const unsigned NodeTextSize = 22;
    inline const unsigned TooltipTextSize = 18;

    // Buttons
    inline const float ButtonW = 320.f;
    inline const float ButtonH = 56.f;
    inline const float ButtonGap = 18.f;

    // Misc numbers
    inline const float Pi = 3.14159265f;
    inline const float BannerH = 340.f;       // selected banner ki height

    // Rang (dark red theme)
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

    // Har act ka aik rang (Briefing panel ke border mein istemal hota hai)
    inline const sf::Color ActColors[4] = 
    {
        sf::Color(170, 40, 50, 200),
        sf::Color(150, 35, 60, 200),
        sf::Color(130, 30, 70, 200),
        sf::Color(190, 50, 40, 200)
    };

    // Framed texture ki woh chaurai jo bina scale hue rehti hai (texture pixels mein).
    // Agar frame ke kone zyada motay ya patle lagen to yeh value badlo.
    inline const int FrameBorderPx = 12;

}  