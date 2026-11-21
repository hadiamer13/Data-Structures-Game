#include "ReportScreen.h"

#include "../Theme.h"
#include "../UIManager.h"
#include "../../game/Campaign.h"

namespace
{
    int maxValue(int a, int b)
    {
        return a > b ? a : b;
    }

    void centerText(sf::Text& text, float x, float y)
    {
        const sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.left + bounds.width * 0.5f,
                       bounds.top + bounds.height * 0.5f);
        text.setPosition(x, y);
    }

    void setText(sf::Text& text, const sf::Font& font, unsigned size,
                 const sf::Color& color, float x, float y)
    {
        text.setFont(font);
        text.setCharacterSize(size);
        text.setFillColor(color);
        text.setPosition(x, y);
    }
}

ReportScreen::ReportScreen(AppContext& ctx) : m_ctx(ctx)
{
    const sf::Font& font = ctx.assets->font();
    const float width = static_cast<float>(Theme::WindowW);
    const float height = static_cast<float>(Theme::WindowH);
    const sf::Texture& background = ctx.assets->campaignBackground();
    m_background.setTexture(background);
    m_background.setScale(width / background.getSize().x, height / background.getSize().y);
    m_dim.setSize(sf::Vector2f(width, height));
    m_dim.setFillColor(sf::Color(0, 0, 0, 175));
    m_panel.setPosition(35.f, 75.f);
    m_panel.setSize(sf::Vector2f(1210.f, 565.f));
    m_panel.setFillColor(sf::Color(12, 8, 14, 230));
    m_panel.setOutlineColor(Theme::BtnOutline);
    m_panel.setOutlineThickness(3.f);

    setText(m_title, font, 40, Theme::TitleColor, 0.f, 92.f);
    m_title.setString("VAULT REPORTS");
    centerText(m_title, Theme::WindowW * 0.5f, 111.f);
    setText(m_campaignHeading, font, 20, Theme::NodeGlow, 88.f, 160.f);
    m_campaignHeading.setString("CAMPAIGN PROGRESS");
    setText(m_scoreHeading, font, 20, Theme::NodeGlow, 665.f, 160.f);
    m_scoreHeading.setString("TOP ENDLESS SCORES");
    setText(m_campaignSummary, font, 17, Theme::TextColor, 88.f, 520.f);
    setText(m_emptyScores, font, 18, Theme::TextColor, 750.f, 330.f);
    m_emptyScores.setString("No saved endless scores yet.");

    for (int i = 0; i < 4; ++i)
    {
        m_actBars[i].setFillColor(Theme::ActColors[i]);
        m_actBars[i].setOutlineColor(Theme::NodeGlow);
        m_actBars[i].setOutlineThickness(1.f);
        setText(m_actLabels[i], font, 15, Theme::TextColor, 0.f, 0.f);
    }
    for (int i = 0; i < Leaderboard::Capacity; ++i)
    {
        m_scoreBars[i].setFillColor(i == 0 ? Theme::StarOn : Theme::NodeLine);
        setText(m_scoreLabels[i], font, 12, Theme::TextColor, 0.f, 0.f);
    }
    m_leaderboard.init("Leaderboard", font, sf::Vector2f(465.f, 600.f), sf::Vector2f(230.f, 48.f));
    m_leaderboard.setCallback(&ReportScreen::onLeaderboard, this);
    m_home.init("Main Menu", font, sf::Vector2f(820.f, 600.f), sf::Vector2f(230.f, 48.f));
    m_home.setCallback(&ReportScreen::onHome, this);
    m_scores.load("saves/leaderboard.json");
    refreshCharts();
}

void ReportScreen::refreshCharts()
{
    int totalStars = 0;
    const float baseY = 465.f;
    for (int act = 0; act < 4; ++act)
    {
        int actStars = 0;
        for (int level = 0; level < 5; ++level)
        {
            if (m_ctx.campaign != nullptr)
            {
                actStars += m_ctx.campaign->stars(act * 5 + level);
            }
        }
        totalStars += actStars;
        const float barHeight = 210.f * static_cast<float>(actStars) / 15.f;
        m_actBars[act].setSize(sf::Vector2f(64.f, barHeight));
        m_actBars[act].setPosition(112.f + act * 125.f, baseY - barHeight);
        m_actLabels[act].setString("Act " + std::to_string(act + 1) + "  " + std::to_string(actStars) + "/15");
    }
    m_campaignSummary.setString("Campaign progress: " + std::to_string(totalStars) + " / 60 stars");

    int maximum = 1;
    for (int i = 0; i < m_scores.size(); ++i)
    {
        maximum = maxValue(maximum, m_scores.at(i).score);
    }
    for (int i = 0; i < Leaderboard::Capacity; ++i)
    {
        if (i >= m_scores.size())
        {
            m_scoreBars[i].setSize(sf::Vector2f(26.f, 0.f));
            m_scoreLabels[i].setString("");
            continue;
        }
        const ScoreEntry& entry = m_scores.at(i);
        const float barHeight = 174.f * static_cast<float>(entry.score) / static_cast<float>(maximum);
        const float centerX = 700.f + i * 51.f;
        m_scoreBars[i].setSize(sf::Vector2f(27.f, barHeight));
        m_scoreBars[i].setOrigin(13.5f, 0.f);
        m_scoreBars[i].setPosition(centerX, baseY - barHeight);
        m_scoreBars[i].setOutlineThickness(i == 0 ? 2.f : 1.f);
        m_scoreBars[i].setOutlineColor(i == 0 ? Theme::StarOn : Theme::BtnOutline);
        m_scoreLabels[i].setString(std::to_string(i + 1));
        centerText(m_scoreLabels[i], centerX, 482.f);
    }
}

void ReportScreen::onLeaderboard(void* context)
{
    static_cast<ReportScreen*>(context)->m_ctx.ui->requestScreen(ScreenId::Leaderboard);
}

void ReportScreen::onHome(void* context)
{
    static_cast<ReportScreen*>(context)->m_ctx.ui->requestScreen(ScreenId::MainMenu);
}

void ReportScreen::handleEvent(const sf::Event& event)
{
    m_leaderboard.handleEvent(event);
    m_home.handleEvent(event);
}

void ReportScreen::update(float dt)
{
    m_leaderboard.update(dt);
    m_home.update(dt);
}

void ReportScreen::draw(sf::RenderWindow& window)
{
    window.draw(m_background);
    window.draw(m_dim);
    window.draw(m_panel);
    window.draw(m_title);
    window.draw(m_campaignHeading);
    window.draw(m_scoreHeading);
    sf::RectangleShape divider(sf::Vector2f(2.f, 340.f));
    divider.setPosition(620.f, 165.f);
    divider.setFillColor(sf::Color(190, 40, 50, 130));
    window.draw(divider);
    sf::RectangleShape campaignBaseline(sf::Vector2f(470.f, 2.f));
    campaignBaseline.setPosition(83.f, 466.f);
    campaignBaseline.setFillColor(Theme::NodeLine);
    window.draw(campaignBaseline);
    sf::RectangleShape scoreBaseline(sf::Vector2f(500.f, 2.f));
    scoreBaseline.setPosition(675.f, 466.f);
    scoreBaseline.setFillColor(Theme::NodeLine);
    window.draw(scoreBaseline);
    for (int i = 0; i < 4; ++i)
    {
        window.draw(m_actBars[i]);
        centerText(m_actLabels[i], m_actBars[i].getPosition().x + 32.f, 486.f);
        window.draw(m_actLabels[i]);
    }
    window.draw(m_campaignSummary);
    if (m_scores.size() == 0)
    {
        window.draw(m_emptyScores);
    }
    for (int i = 0; i < m_scores.size(); ++i)
    {
        window.draw(m_scoreBars[i]);
        window.draw(m_scoreLabels[i]);
    }
    m_leaderboard.draw(window);
    m_home.draw(window);
}
