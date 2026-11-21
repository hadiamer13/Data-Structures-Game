#pragma once
#include "Screen.h"
#include "../AppContext.h"
#include "../widgets/Button.h"

// main menu
// Game ka pehla screen. Campaign, Endless, Sandbox, Leaderboard, Settings aur Quit ke buttons.
class MainMenu : public Screen 
{
public:
    explicit MainMenu(AppContext& ctx);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    // Static callback se call hota hai, isliye public hai.
    void onPressed(int target);

private:
    static const int ButtonCount = 6;

    // Har button ka alag context hota hai, taake aik hi static function sab ko handle kar sake.
    struct ButtonCtx {
        MainMenu* menu;
        int       target;   // ScreenId value, ya -2 jiska matlab quit hai
    };
    static void onButtonClick(void* context);

    AppContext& m_ctx;
    sf::Sprite  m_background;
    sf::Text    m_title;
    sf::Text    m_subtitle;
    Button      m_buttons[ButtonCount];
    ButtonCtx   m_btnCtx[ButtonCount];
};