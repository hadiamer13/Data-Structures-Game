// gui/Assets.cpp
#include "Assets.h"
#include "Theme.h"
#include <cstdio>

namespace {

    // Loads one texture, smooths it for scaling, and reports failures by path.
    bool loadTexture(sf::Texture& tex, const char* path) {
        if (!tex.loadFromFile(path)) {
            std::fprintf(stderr, "[Assets] Failed to load texture: %s\n", path);
            return false;
        }
        tex.setSmooth(true);
        return true;
    }

}  // namespace

bool Assets::loadAll() {
    bool ok = true;

    // Required: the game cannot run without these.
    if (!m_font.loadFromFile(Theme::FontPath)) {
        std::fprintf(stderr, "[Assets] Failed to load font: %s\n", Theme::FontPath);
        ok = false;
    }
    ok = loadTexture(m_menuBg, Theme::MenuBgPath) && ok;
    ok = loadTexture(m_campaignBg, Theme::CampaignBgPath) && ok;
    ok = loadTexture(m_banner, Theme::BannerPath) && ok;

    // Optional: decorative pieces. A missing file only prints a warning,
    // and setFrame / setRingTexture / placeCorner fall back to plain drawing.
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