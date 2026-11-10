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

}  // namespace Theme