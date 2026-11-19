#pragma once

#include "Screen.h"
#include "../AppContext.h"
#include "../widgets/Button.h"

class SettingsScreen : public Screen
{
public:
    explicit SettingsScreen(AppContext& ctx);
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    struct ButtonContext
    {
        SettingsScreen* owner;
        int action;
    };
    static void onButton(void* context);
    void change(int setting, int direction);
    AppContext& m_ctx;
    sf::Sprite m_background;
    sf::RectangleShape m_dim;
    sf::RectangleShape m_panel;
    sf::Text m_title;
    sf::Text m_antialiasing;
    sf::Text m_speed;
    sf::Text m_help;
    Button m_buttons[5];
    ButtonContext m_buttonContexts[5];
};
