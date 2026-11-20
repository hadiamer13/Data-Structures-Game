// gui/Assets.cpp
#include "Assets.h"
#include "Theme.h"
#include <cstdio>

namespace {

    // Aik texture load karta hai, scaling ke liye smooth karta hai, aur failure par path print karta hai.
    bool loadTexture(sf::Texture& tex, const char* path) {
        if (!tex.loadFromFile(path)) {
            std::fprintf(stderr, "[Assets] Texture load nahi hui: %s\n", path);
            return false;
        }
        tex.setSmooth(true);
        return true;
    }

}  // namespace

bool Assets::loadAll() {
    bool ok = true;

    // Zaroori assets: in ke bina game nahi chal sakti.
    if (!m_font.loadFromFile(Theme::FontPath)) {
        std::fprintf(stderr, "[Assets] Font load nahi hua: %s\n", Theme::FontPath);
        ok = false;
    }
    ok = loadTexture(m_menuBg, Theme::MenuBgPath) && ok;
    ok = loadTexture(m_campaignBg, Theme::CampaignBgPath) && ok;
    ok = loadTexture(m_banner, Theme::BannerPath) && ok;

    // Optional decorative cheezein. Agar file missing ho to sirf warning aayegi,
    // aur setFrame / setRingTexture / placeCorner saada drawing kar lenge.
    loadTexture(m_skull, Theme::SkullPath);
    loadTexture(m_nodeRing, Theme::NodeRingPath);
    loadTexture(m_buttonFrame, Theme::ButtonFramePath);
    loadTexture(m_divider, Theme::DividerPath);
    loadTexture(m_dividerAlt, Theme::DividerAltPath);
    loadTexture(m_highlight, Theme::HighlightPath);
    loadTexture(m_corner, Theme::CornerPath);
    loadTexture(m_cornerSmall, Theme::CornerSmallPath);

    return ok;
}