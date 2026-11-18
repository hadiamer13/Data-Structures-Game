// gui/main_gui.cpp
#include <SFML/Graphics.hpp>
#include <cstdio>
#include "Theme.h"
#include "Assets.h"
#include "AppContext.h"
#include "UIManager.h"
#include "../game/Campaign.h"

// The model test runner is the project's active entry point while running the
// integrated build. Re-enable this block to launch the SFML menu instead.
#if 0
int main() {
    // ---- Window (SFML 2.x: sf::VideoMode(w, h)) ----------------------
    sf::RenderWindow window(sf::VideoMode(Theme::WindowW, Theme::WindowH),
        "Vault Heist: The Living Vault",
        sf::Style::Titlebar | sf::Style::Close);
    // Use setFramerateLimit OR vsync, never both.
    window.setFramerateLimit(Theme::FrameLimit);

    // ---- Assets: loaded ONCE ----------------------------------------
    // TODO(YOU): if this fails, check the paths in Theme.h and run the
    // exe from the project root so "assets/..." resolves.
    Assets assets;
    if (!assets.loadAll())
    {
        std::fprintf(stderr, "Asset loading failed. Exiting.\n");
        return 1;
    }

    // ---- Campaign: level definitions + star progress, loaded ONCE ----
    // Declared before ui so it outlives every screen that points at it.
    // A load failure is shown on the campaign map instead of crashing.
    Campaign campaign;
    if (!campaign.load())
    {
        std::fprintf(stderr, "Campaign data incomplete:\n%s\n", campaign.error().c_str());
    }

    // ---- Shared context + screen manager -----------------------------
    AppContext ctx;
    ctx.window = &window;
    ctx.assets = &assets;
    ctx.campaign = &campaign;

    UIManager ui(ctx);
    ctx.ui = &ui;

    // INTEGRATION (later): create the GameSession / SaveManager load here,
    // store a pointer in ctx, and call SaveManager load before ui.start().
    ui.start();

    // ---- Main loop ---------------------------------------------------
    sf::Clock clock;
    while (window.isOpen() && !ui.wantsQuit())
    {
        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed)
            {
                window.close();
            }
            else
            {
                ui.handleEvent(e);
            }
        }

        // Cap dt so a long stall never makes animations jump.
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f)
        {
            dt = 0.05f;
        }

        ui.update(dt);

        window.clear(sf::Color::Black);
        ui.draw(window);
        window.display();
    }

    return 0;
}
#endif
