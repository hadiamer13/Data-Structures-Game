#include "SettingsScreen.h"

#include <string>

#include "../Theme.h"
#include "../UIManager.h"

namespace
{
    void setText(sf::Text& text, const sf::Font& font, unsigned size,
        const sf::Color& color, float x, float y)
    {
        text.setFont(font);
        text.setCharacterSize(size);
        text.setFillColor(color);
        text.setPosition(x, y);
    }

    std::string speedText(float speed)
    {
        if (speed == static_cast<int>(speed))
        {
            return std::to_string(static_cast<int>(speed)) + ".0x";
        }
        return std::to_string(speed).substr(0, 4) + "x";
    }
}

SettingsScreen::SettingsScreen(AppContext& ctx) : m_ctx(ctx)
{
    const sf::Font& font = ctx.assets->font();
    const float width = static_cast<float>(Theme::WindowW);
    const float height = static_cast<float>(Theme::WindowH);
    const sf::Texture& background = ctx.assets->campaignBackground();
    m_background.setTexture(background);
    m_background.setScale(width / background.getSize().x, height / background.getSize().y);
    m_dim.setSize(sf::Vector2f(width, height));
    m_dim.setFillColor(sf::Color(0, 0, 0, 182));
    m_panel.setPosition(205.f, 88.f);
    m_panel.setSize(sf::Vector2f(870.f, 545.f));
    m_panel.setFillColor(sf::Color(12, 8, 14, 230));
    m_panel.setOutlineColor(Theme::BtnOutline);
    m_panel.setOutlineThickness(3.f);

    setText(m_title, font, 44, Theme::TitleColor, 530.f, 112.f);
    m_title.setString("SETTINGS");
    setText(m_antialiasing, font, 25, Theme::TextColor, 270.f, 240.f);
    setText(m_speed, font, 25, Theme::TextColor, 270.f, 375.f);
    setText(m_help, font, 16, Theme::NodeGlow, 270.f, 470.f);
    m_help.setString("Display changes apply immediately. Animation speed affects game playback.");

    const char* labels[5] = { "-", "+", "-", "+", "Main Menu" };
    const float xs[5] = { 850.f, 1000.f, 850.f, 1000.f, 640.f };
    const float ys[5] = { 250.f, 250.f, 385.f, 385.f, 570.f };
    const float ws[5] = { 68.f, 68.f, 68.f, 68.f, 220.f };
    for (int i = 0; i < 5; ++i)
    {
        m_buttonContexts[i].owner = this;
        m_buttonContexts[i].action = i;
        m_buttons[i].init(labels[i], font, sf::Vector2f(xs[i], ys[i]), sf::Vector2f(ws[i], 48.f));
        m_buttons[i].setCallback(&SettingsScreen::onButton, &m_buttonContexts[i]);
    }
}

void SettingsScreen::onButton(void* context)
{
    ButtonContext* button = static_cast<ButtonContext*>(context);
    if (button->action == 0) button->owner->change(0, -1);
    else if (button->action == 1) button->owner->change(1, 1);
    else if (button->action == 2) button->owner->change(2, -1);
    else if (button->action == 3) button->owner->change(3, 1);
    else button->owner->change(4, 0);
}

void SettingsScreen::change(int setting, int direction)
{
    if (setting == 4)
    {
        m_ctx.ui->requestScreen(ScreenId::MainMenu);
        return;
    }
    if (m_ctx.settings == nullptr) return;
    if (setting < 2)
    {
        const unsigned int levels[4] = { 0, 2, 4, 8 };
        int index = 0;
        while (index < 3 && levels[index] < m_ctx.settings->antialiasingLevel) ++index;
        index += direction;
        if (index < 0) index = 0;
        if (index > 3) index = 3;
        if (levels[index] != m_ctx.settings->antialiasingLevel)
        {
            m_ctx.settings->antialiasingLevel = levels[index];
            m_ctx.settings->recreateWindow = true;
        }
    }
    else
    {
        float next = m_ctx.settings->animationSpeed + 0.25f * direction;
        if (next < 0.5f) next = 0.5f;
        if (next > 2.f) next = 2.f;
        m_ctx.settings->animationSpeed = next;
    }
}

void SettingsScreen::handleEvent(const sf::Event& event)
{
    for (int i = 0; i < 5; ++i) m_buttons[i].handleEvent(event);
}

void SettingsScreen::update(float dt)
{
    (void)dt;
    const unsigned int antialiasing = m_ctx.settings == nullptr ? 0 : m_ctx.settings->antialiasingLevel;
    const float speed = m_ctx.settings == nullptr ? 1.f : m_ctx.settings->animationSpeed;
    const std::string antialiasingLabel = "Antialiasing level:  " + std::to_string(antialiasing);
    if (m_antialiasing.getString().toAnsiString() != antialiasingLabel)
    {
        m_antialiasing.setString(antialiasingLabel);
    }
    const std::string speedLabel = "Animation speed:  " + speedText(speed);
    if (m_speed.getString().toAnsiString() != speedLabel)
    {
        m_speed.setString(speedLabel);
    }
    for (int i = 0; i < 5; ++i) m_buttons[i].update(dt);
}

void SettingsScreen::draw(sf::RenderWindow& window)
{
    window.draw(m_background);
    window.draw(m_dim);
    window.draw(m_panel);
    sf::RectangleShape accent(sf::Vector2f(5.f, 56.f));
    accent.setFillColor(Theme::NodeGlow);
    accent.setPosition(254.f, 230.f);
    window.draw(accent);
    accent.setPosition(254.f, 365.f);
    window.draw(accent);
    sf::RectangleShape divider(sf::Vector2f(770.f, 2.f));
    divider.setPosition(255.f, 325.f);
    divider.setFillColor(sf::Color(190, 40, 50, 125));
    window.draw(divider);
    sf::RectangleShape track(sf::Vector2f(300.f, 8.f));
    track.setFillColor(sf::Color(70, 45, 50));
    track.setPosition(350.f, 530.f);
    window.draw(track);
    sf::RectangleShape fill(sf::Vector2f(150.f, 8.f));
    fill.setFillColor(Theme::NodeGlow);
    fill.setPosition(350.f, 530.f);
    if (m_ctx.settings != nullptr)
    {
        float progress = (m_ctx.settings->animationSpeed - 0.5f) / 1.5f;
        if (progress < 0.f) progress = 0.f;
        if (progress > 1.f) progress = 1.f;
        fill.setSize(sf::Vector2f(300.f * progress, 8.f));
    }
    window.draw(fill);
    window.draw(m_title);
    window.draw(m_antialiasing);
    window.draw(m_speed);
    window.draw(m_help);
    for (int i = 0; i < 5; ++i) m_buttons[i].draw(window);
}
