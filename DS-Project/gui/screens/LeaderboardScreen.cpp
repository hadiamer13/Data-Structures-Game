#include "LeaderboardScreen.h"

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
}

LeaderboardScreen::LeaderboardScreen(AppContext& ctx) : m_ctx(ctx), m_loaded(false)
{
    const sf::Font& font = ctx.assets->font();
    const float width = static_cast<float>(Theme::WindowW);
    const float height = static_cast<float>(Theme::WindowH);
    const sf::Texture& background = ctx.assets->campaignBackground();
    m_background.setTexture(background);
    m_background.setScale(width / background.getSize().x, height / background.getSize().y);
    m_dim.setSize(sf::Vector2f(width, height));
    m_dim.setFillColor(sf::Color(0, 0, 0, 185));
    m_panel.setPosition(180.f, 68.f);
    m_panel.setSize(sf::Vector2f(920.f, 585.f));
    m_panel.setFillColor(sf::Color(12, 8, 14, 230));
    m_panel.setOutlineColor(Theme::BtnOutline);
    m_panel.setOutlineThickness(3.f);

    setText(m_title, font, 40, Theme::TitleColor, 430.f, 88.f);
    m_title.setString("LOCAL LEADERBOARD");
    setText(m_empty, font, 21, Theme::TextColor, 390.f, 330.f);
    m_empty.setString("No endless scores yet. Survive a round to set a record.");
    setText(m_error, font, 14, Theme::NodeGlow, 260.f, 570.f);
    for (int i = 0; i < Leaderboard::Capacity; ++i)
    {
        setText(m_rows[i], font, 21,
                i == 0 ? Theme::StarOn : Theme::TextColor,
                305.f, 180.f + static_cast<float>(i) * 37.f);
    }

    m_home.init("Main Menu", font, sf::Vector2f(415.f, 610.f), sf::Vector2f(220.f, 50.f));
    m_home.setCallback(&LeaderboardScreen::onHome, this);
    m_reports.init("View Reports", font, sf::Vector2f(865.f, 610.f), sf::Vector2f(240.f, 50.f));
    m_reports.setCallback(&LeaderboardScreen::onReports, this);
    std::string error;
    m_loaded = m_scores.load("saves/leaderboard.json", &error);
    if (!m_loaded && error.find("Unable to open leaderboard") == std::string::npos)
    {
        m_error.setString(error);
    }
}

void LeaderboardScreen::onHome(void* context)
{
    static_cast<LeaderboardScreen*>(context)->m_ctx.ui->requestScreen(ScreenId::MainMenu);
}

void LeaderboardScreen::onReports(void* context)
{
    static_cast<LeaderboardScreen*>(context)->m_ctx.ui->requestScreen(ScreenId::Reports);
}

void LeaderboardScreen::handleEvent(const sf::Event& event)
{
    m_home.handleEvent(event);
    m_reports.handleEvent(event);
}

void LeaderboardScreen::update(float dt)
{
    m_home.update(dt);
    m_reports.update(dt);
    for (int i = 0; i < Leaderboard::Capacity; ++i)
    {
        if (i < m_scores.size())
        {
            const ScoreEntry& entry = m_scores.at(i);
            std::string name = entry.name;
            if (name.size() > 20)
            {
                name = name.substr(0, 18) + "..";
            }
            m_rows[i].setString("#" + std::to_string(i + 1) + "     " + name +
                                "                         " + std::to_string(entry.score) + " pts");
        }
        else
        {
            m_rows[i].setString("");
        }
    }
}

void LeaderboardScreen::draw(sf::RenderWindow& window)
{
    window.draw(m_background);
    window.draw(m_dim);
    window.draw(m_panel);
    sf::RectangleShape headerRule(sf::Vector2f(820.f, 2.f));
    headerRule.setPosition(230.f, 145.f);
    headerRule.setFillColor(Theme::NodeLine);
    window.draw(headerRule);
    for (int i = 0; i < Leaderboard::Capacity; ++i)
    {
        sf::RectangleShape row(sf::Vector2f(820.f, 32.f));
        row.setPosition(230.f, 172.f + static_cast<float>(i) * 37.f);
        const sf::Uint8 alpha = (i % 2 == 0) ? 48 : 24;
        row.setFillColor(sf::Color(110, 20, 30, alpha));
        if (i == 0)
        {
            row.setOutlineThickness(1.f);
            row.setOutlineColor(sf::Color(255, 205, 80, 150));
        }
        window.draw(row);
    }
    window.draw(m_title);
    if (m_scores.size() == 0)
    {
        window.draw(m_empty);
    }
    for (int i = 0; i < Leaderboard::Capacity; ++i)
    {
        window.draw(m_rows[i]);
    }
    window.draw(m_error);
    m_home.draw(window);
    m_reports.draw(window);
}
