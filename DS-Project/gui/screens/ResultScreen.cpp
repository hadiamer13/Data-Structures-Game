// gui/screens/ResultScreen.cpp
#include "ResultScreen.h"
#include "../UIManager.h"
#include "../Theme.h"
#include "../../game/Campaign.h"
#include <cmath>

namespace
{
    void styleText(sf::Text& t, const sf::Font& font, unsigned size, const sf::Color& col)
    {
        t.setFont(font);
        t.setCharacterSize(size);
        t.setFillColor(col);
        t.setOutlineColor(sf::Color(0, 0, 0, 200));
        t.setOutlineThickness(2.f);
    }

    void centreAt(sf::Text& t, float x, float y)
    {
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
        t.setPosition(x, y);
    }
}

ResultScreen::ResultScreen(AppContext& ctx) : m_ctx(ctx)
{
    const sf::Font& font = ctx.assets->font();
    const LevelResult& r = ctx.result;
    const float W = static_cast<float>(Theme::WindowW);
    const float H = static_cast<float>(Theme::WindowH);
    const float cx = W / 2.f;

    m_won = r.won;
    int idx = r.levelIndex;
    if (idx < 0 || idx > 19)
    {
        idx = 0;
    }
    m_lastLevel = (idx >= 19);
    m_showBadge = (r.won && r.heapAct && idx == 19);

    // Save this result into campaign progress. setStars keeps the best value,
    // so showing the same result again is harmless.
    if (ctx.campaign != nullptr)
    {
        ctx.campaign->setStars(idx, r.stars);
    }

    const sf::Texture& bg = ctx.assets->campaignBackground();
    m_background.setTexture(bg);
    m_background.setScale(W / static_cast<float>(bg.getSize().x), H / static_cast<float>(bg.getSize().y));
    m_dim.setSize(sf::Vector2f(W, H));
    m_dim.setFillColor(sf::Color(0, 0, 0, 0)); // fades in via update()

    const float pw = 760.f;
    const float ph = 540.f;
    const float px = cx - pw / 2.f;
    const float py = 80.f;
    m_panel.setSize(sf::Vector2f(pw, ph));
    m_panel.setPosition(px, py);
    m_panel.setFillColor(Theme::TooltipBg);
    m_panel.setOutlineColor(r.won ? Theme::StarOn : Theme::NodeLine);
    m_panel.setOutlineThickness(r.won ? 6.f : 3.f); // not colour alone

    // Status is a word, not only a colour.
    styleText(m_title, font, 44, r.won ? Theme::StarOn : Theme::TitleColor);
    m_title.setString(r.won ? "LEVEL CLEARED" : "ALARM FULL - LEVEL FAILED");
    centreAt(m_title, cx, py + 42.f);

    const int act = idx / 5 + 1;
    const int lvl = idx % 5 + 1;
    styleText(m_sub, font, 20, Theme::NodeGlow);
    m_sub.setString("ACT " + std::to_string(act) + "  -  LEVEL " + std::to_string(act) + "." + std::to_string(lvl));
    centreAt(m_sub, cx, py + 92.f);

    // stars
    const float outer = 28.f;
    const float inner = outer * 0.45f;
    for (int s = 0; s < 3; ++s)
    {
        m_star[s].setPointCount(10);
        for (int p = 0; p < 10; ++p)
        {
            float rad = (p % 2 == 0) ? outer : inner;
            float a = -Theme::Pi / 2.f + static_cast<float>(p) * (Theme::Pi / 5.f);
            m_star[s].setPoint(p, sf::Vector2f(rad * std::cos(a), rad * std::sin(a)));
        }
        m_star[s].setPosition(cx + static_cast<float>(s - 1) * 80.f, py + 150.f);
        m_star[s].setOutlineThickness(2.f);
        bool on = s < r.stars;
        m_star[s].setFillColor(on ? Theme::StarOn : sf::Color(0, 0, 0, 0));
        m_star[s].setOutlineColor(on ? Theme::StarOn : Theme::StarOff);
    }

    styleText(m_stats, font, 26, Theme::TextColor);
    m_stats.setString("Steps: " + std::to_string(r.alarm) + "   (optimal " + std::to_string(r.optimal) + ")" + "\nPenalties: " + std::to_string(r.penalties) + "\nScore: " + std::to_string(r.score) + "\nStars: " + std::to_string(r.stars) + " / 3");
    m_stats.setLineSpacing(1.15f);
    centreAt(m_stats, cx, py + 270.f);

    // Act 4: final array, 12 values per line
    styleText(m_sorted, font, 18, Theme::TextColor);
    if (r.heapAct && r.sortedArray.size() > 0)
    {
        std::string s = "SORTED ARRAY:\n";
        for (int i = 0; i < r.sortedArray.size(); ++i)
        {
            s += std::to_string(r.sortedArray[i]);
            if (i + 1 < r.sortedArray.size())
            {
                s += ((i + 1) % 12 == 0) ? "\n" : "  ";
            }
        }
        m_sorted.setString(s);
        centreAt(m_sorted, cx, py + 390.f);
    }

    // Completion badge (text + gold frame)
    styleText(m_badge, font, 26, Theme::StarOn);
    m_badge.setString("FINAL VAULT UNLOCKED");
    centreAt(m_badge, cx, py + 452.f);
    m_badgeBox.setSize(sf::Vector2f(380.f, 44.f));
    m_badgeBox.setOrigin(190.f, 22.f);
    m_badgeBox.setPosition(cx, py + 452.f);
    m_badgeBox.setFillColor(sf::Color(0, 0, 0, 0));
    m_badgeBox.setOutlineColor(Theme::StarOn);
    m_badgeBox.setOutlineThickness(3.f);

    // buttons
    const float by = py + ph - 38.f;
    const sf::Vector2f bs(220.f, Theme::ButtonH);
    if (m_won)
    {
        m_retry.init("Retry", font, sf::Vector2f(cx - 250.f, by), bs);
        m_next.init(m_lastLevel ? "Main Menu" : "Next Level", font, sf::Vector2f(cx, by), bs);
        m_campaign.init("Campaign", font, sf::Vector2f(cx + 250.f, by), bs);
    }
    else
    {
        m_retry.init("Retry", font, sf::Vector2f(cx - 130.f, by), bs);
        m_campaign.init("Campaign", font, sf::Vector2f(cx + 130.f, by), bs);
    }
    m_retry.setCallback(&ResultScreen::onRetryClick, this);
    m_next.setCallback(&ResultScreen::onNextClick, this);
    m_campaign.setCallback(&ResultScreen::onCampaignClick, this);

    m_retry.setFrame(&ctx.assets->buttonFrame());
    m_next.setFrame(&ctx.assets->buttonFrame());
    m_campaign.setFrame(&ctx.assets->buttonFrame());
}

void ResultScreen::onRetryClick(void* context)
{
    static_cast<ResultScreen*>(context)->onRetry();
}

void ResultScreen::onNextClick(void* context)
{
    static_cast<ResultScreen*>(context)->onNext();
}

void ResultScreen::onCampaignClick(void* context)
{
    static_cast<ResultScreen*>(context)->onCampaign();
}

void ResultScreen::onRetry()
{
    m_ctx.selectedLevel = m_ctx.result.levelIndex;
    m_ctx.ui->requestScreen(m_ctx.result.levelIndex >= 15 ? ScreenId::HeapLevel : ScreenId::Level);
}

void ResultScreen::onNext()
{
    if (m_lastLevel)
    {
        m_ctx.ui->requestScreen(ScreenId::MainMenu);
        return;
    }
    m_ctx.selectedLevel = m_ctx.result.levelIndex + 1;
    m_ctx.ui->requestScreen(ScreenId::Briefing);
}

void ResultScreen::onCampaign()
{
    m_ctx.selectedLevel = m_ctx.result.levelIndex; // Campaign reopens on this level
    m_ctx.ui->requestScreen(ScreenId::Campaign);
}

void ResultScreen::handleEvent(const sf::Event& e)
{
    // Esc is handled by UIManager (goes to the Campaign).
    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Return)
    {
        if (m_won)
        {
            onNext();
        }
        else
        {
            onRetry();
        }
        return;
    }
    m_retry.handleEvent(e);
    m_campaign.handleEvent(e);
    if (m_won)
    {
        m_next.handleEvent(e);
    }
}

void ResultScreen::update(float dt)
{
    m_retry.update(dt);
    m_campaign.update(dt);
    m_next.update(dt);

    if (m_fade < 1.f)
    {
        m_fade += dt * 4.f;
        if (m_fade > 1.f)
        {
            m_fade = 1.f;
        }
        m_dim.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(180.f * m_fade)));
    }
}

void ResultScreen::draw(sf::RenderWindow& window)
{
    window.draw(m_background);
    window.draw(m_dim);
    window.draw(m_panel);
    window.draw(m_title);
    window.draw(m_sub);
    for (int s = 0; s < 3; ++s)
    {
        window.draw(m_star[s]);
    }
    window.draw(m_stats);
    window.draw(m_sorted);
    if (m_showBadge)
    {
        window.draw(m_badgeBox);
        window.draw(m_badge);
    }
    m_retry.draw(window);
    if (m_won)
    {
        m_next.draw(window);
    }
    m_campaign.draw(window);
}