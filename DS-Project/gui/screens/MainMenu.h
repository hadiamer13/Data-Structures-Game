// gui/screens/MainMenu.h
#pragma once
#include "Screen.h"
#include "../AppContext.h"
#include "../widgets/Button.h"

class MainMenu : public Screen {
public:
    explicit MainMenu(AppContext& ctx);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    // Called by the button callback (public so the static function can reach it).
    void onPressed(int target);

private:
    static const int ButtonCount = 6;

    // One context object per button, so a single static callback can tell
    // which button was clicked (no std::function, no lambdas needed).
    struct ButtonCtx {
        MainMenu* menu;
        int       target;   // ScreenId value, or -2 meaning "quit"
    };
    static void onButtonClick(void* context);

    AppContext& m_ctx;
    sf::Sprite  m_background;
    sf::Text    m_title;
    sf::Text    m_subtitle;
    Button      m_buttons[ButtonCount];
    ButtonCtx   m_btnCtx[ButtonCount];
};