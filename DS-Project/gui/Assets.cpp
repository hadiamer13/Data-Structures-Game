// gui/Assets.cpp
#include "Assets.h"
#include "Theme.h"
#include <cstdio>

bool Assets::loadAll() {
    bool ok = true;

    // TODO(YOU): make sure this file exists at assets/fonts/vault.ttf
    if (!m_font.loadFromFile(Theme::FontPath)) {
        std::fprintf(stderr, "[Assets] Failed to load font: %s\n",
            Theme::FontPath);
        ok = false;
    }

    // TODO(YOU): make sure this file exists at assets/images/menu_bg.png
    if (!m_menuBg.loadFromFile(Theme::MenuBgPath)) {
        std::fprintf(stderr, "[Assets] Failed to load menu background: %s\n",
            Theme::MenuBgPath);
        ok = false;
    }
    else {
        m_menuBg.setSmooth(true);
    }

    // TODO(YOU): make sure this file exists at assets/images/campaign_bg.png
    if (!m_campaignBg.loadFromFile(Theme::CampaignBgPath)) {
        std::fprintf(stderr, "[Assets] Failed to load campaign background: %s\n",
            Theme::CampaignBgPath);
        ok = false;
    }
    else {
        m_campaignBg.setSmooth(true);
    }

    // TODO(YOU, later): load more images here, e.g.
    // m_campaignBg.loadFromFile("assets/images/campaign_bg.png");
    //   m_vaultDoor.loadFromFile("assets/images/vault_door.png");
    // and add matching accessors in Assets.h.

    return ok;
}