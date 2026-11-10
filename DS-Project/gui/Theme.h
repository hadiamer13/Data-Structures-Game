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

	// ---- Fonts ----
	inline const unsigned TitleSize = 64;
	inline const unsigned ButtonTextSize = 26;

	// ---- Buttons ----
	inline const float ButtonW = 320.f;
	inline const float ButtonH = 56.f;
	inline const float ButtonGap = 18.f;

	// ---- Colours ----
	inline const sf::Color BtnIdle(30, 30, 40, 210);
	inline const sf::Color BtnHover(70, 60, 110, 230);
	inline const sf::Color BtnPressed(20, 20, 30, 240);
	inline const sf::Color BtnOutline(200, 170, 80);     // gold
	inline const sf::Color TextColor(240, 235, 220);
	inline const sf::Color TitleColor(230, 190, 90);


	// ---- Campaign map ----
	// TODO(YOU): put your campaign background image here
	inline const char* const CampaignBgPath = "assets/images/campaign_bg.png";

	inline const float NodeRadius = 30.f;
	inline const float StarRadius = 9.f;
	inline const unsigned NodeTextSize = 22;
	inline const unsigned TooltipTextSize = 18;
	inline const float Pi = 3.14159265f;

	inline const sf::Color NodeLocked(60, 60, 70, 230);
	inline const sf::Color NodeLockedLine(130, 130, 140);
	inline const sf::Color NodeUnlocked(45, 40, 75, 235);
	inline const sf::Color NodeDone(95, 70, 150, 240);
	inline const sf::Color NodeLine(200, 170, 80);      // gold
	inline const sf::Color NodeGlow(255, 220, 120);
	inline const sf::Color StarOn(255, 215, 60);
	inline const sf::Color StarOff(80, 80, 90);
	inline const sf::Color TooltipBg(15, 15, 25, 235);

	// One colour per act (used by banners in CampaignMap)
	inline const sf::Color ActColors[4] = {
		sf::Color(60, 120, 170, 200),   // Act 1
		sf::Color(70, 140, 90, 200),    // Act 2
		sf::Color(150, 110, 60, 200),   // Act 3
		sf::Color(150, 70, 70, 200)     // Act 4
	};


}  // namespace Theme