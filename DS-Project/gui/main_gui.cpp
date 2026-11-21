// gui/main_gui.cpp
// Yahan se game shuru hoti hai: window, assets, campaign data aur main loop.
#include <SFML/Graphics.hpp>
#include <cstdio>
#include "Theme.h"
#include "Assets.h"
#include "AppContext.h"
#include "UIManager.h"
#include "../game/Campaign.h"

// Model test runner abhi project ka active entry point hai jab integrated build chalta hai.

int main() {
    // Window (SFML 2.x: sf::VideoMode(w, h))
    GameSettings gameSettings;
    sf::ContextSettings contextSettings;
    contextSettings.antialiasingLevel = gameSettings.antialiasingLevel;
    sf::RenderWindow window(sf::VideoMode(Theme::WindowW, Theme::WindowH),
        "Vault Heist: The Living Vault",
        sf::Style::Titlebar | sf::Style::Close, contextSettings);
    // setFramerateLimit ya vsync: dono ek saath nahi.
    window.setFramerateLimit(Theme::FrameLimit);

    // Assets sirf aik baar load hote hain.
    // Agar yeh fail ho to paths Theme.h mein check karo aur exe project root se chalao
    // taake "assets/..." sahi resolve ho.
    Assets assets;
    if (!assets.loadAll())
    {
        std::fprintf(stderr, "Assets load nahi hue. Band ho raha hai.\n");
        return 1;
    }

    // Campaign: level definitions aur stars ki progress, sirf aik baar load.
    // Yeh UIManager se pehle declare hota hai taake har screen se zyada zinda rahe.
    // Load fail ho to campaign map par error dikhta hai, crash nahi hota.
    Campaign campaign;
    if (!campaign.load())
    {
        std::fprintf(stderr, "Campaign data adhoora hai:\n%s\n", campaign.error().c_str());
    }

    // Shared context aur screen manager
    AppContext ctx;
    ctx.window = &window;
    ctx.assets = &assets;
    ctx.campaign = &campaign;
    ctx.settings = &gameSettings;

    UIManager ui(ctx);
    ctx.ui = &ui;

    ui.start();

    // Main loop
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

        // dt ko cap karte hain taake lamba stall hone par animation ek dam na chale.
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f)
        {
            dt = 0.05f;
        }
        if (ctx.settings != nullptr)
        {
            dt *= ctx.settings->animationSpeed;
        }

        ui.update(dt);

        if (gameSettings.recreateWindow)
        {
            gameSettings.recreateWindow = false;
            sf::ContextSettings changedSettings;
            changedSettings.antialiasingLevel = gameSettings.antialiasingLevel;
            window.create(sf::VideoMode(Theme::WindowW, Theme::WindowH),
                "Vault Heist: The Living Vault",
                sf::Style::Titlebar | sf::Style::Close, changedSettings);
            window.setFramerateLimit(Theme::FrameLimit);
        }

        window.clear(sf::Color::Black);
        ui.draw(window);
        window.display();
    }

    return 0;
}