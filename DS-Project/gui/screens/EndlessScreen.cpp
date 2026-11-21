#include "EndlessScreen.h"

#include <filesystem>
#include <string>

#include "../Theme.h"
#include "../UIManager.h"
#include "../../game/Campaign.h"
#include "../../game/GameSession.h"

namespace
{
    int mistakePenalty(int score)
    {
        const int penalty = 5 + score / 500;
        return penalty > 15 ? 15 : penalty;
    }

    std::string wrapText(const std::string& value, int lineWidth)
    {
        std::string result;
        int lineLength = 0;
        for (size_t i = 0; i < value.size(); ++i)
        {
            const char ch = value[i];
            if (ch == ' ' && lineLength >= lineWidth)
            {
                result += '\n';
                lineLength = 0;
            }
            else
            {
                result += ch;
                ++lineLength;
            }
        }
        return result;
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

EndlessScreen::EndlessScreen(AppContext& ctx)
    : m_ctx(ctx), m_renderer(ctx.assets->font()), m_seconds(60.f), m_score(0),
      m_expectedOperation(SandboxOp::Insert), m_expectedKey(0), m_random(73291u),
      m_gameOver(false), m_scoreSaved(false)
{
    const sf::Font& font = ctx.assets->font();
    const float width = static_cast<float>(Theme::WindowW);
    const float height = static_cast<float>(Theme::WindowH);
    const sf::Texture& background = ctx.assets->campaignBackground();
    m_background.setTexture(background);
    m_background.setScale(width / background.getSize().x, height / background.getSize().y);

    m_dim.setSize(sf::Vector2f(width, height));
    m_dim.setFillColor(sf::Color(0, 0, 0, 182));
    m_panel.setPosition(24.f, 76.f);
    m_panel.setSize(sf::Vector2f(432.f, 600.f));
    m_panel.setFillColor(sf::Color(12, 8, 14, 230));
    m_panel.setOutlineColor(Theme::BtnOutline);
    m_panel.setOutlineThickness(2.f);

    setText(m_title, font, 30, Theme::TitleColor, 50.f, 100.f);
    m_title.setString("ENDLESS SURVIVAL");
    setText(m_prompt, font, 23, Theme::TextColor, 52.f, 205.f);
    setText(m_status, font, 17, Theme::NodeGlow, 52.f, 315.f);
    setText(m_scoreText, font, 21, Theme::StarOn, 52.f, 150.f);
    setText(m_timerText, font, 21, Theme::TextColor, 286.f, 150.f);
    setText(m_hint, font, 14, Theme::TextColor, 52.f, 375.f);

    m_keyField.init("KEY", font, 76.f, 445.f, 280.f, 44.f);
    const int operations[3] = {SandboxOp::Insert, SandboxOp::Search, SandboxOp::Delete};
    const char* labels[3] = {"Insert", "Search", "Delete"};
    const float centres[3] = {108.f, 242.f, 376.f};
    for (int i = 0; i < 3; ++i)
    {
        m_actionContexts[i].owner = this;
        m_actionContexts[i].operation = operations[i];
        m_actions[i].init(labels[i], font, sf::Vector2f(centres[i], 552.f), sf::Vector2f(112.f, 48.f));
        m_actions[i].setCallback(&EndlessScreen::onAction, &m_actionContexts[i]);
    }
    m_home.init("Main Menu", font, sf::Vector2f(148.f, 632.f), sf::Vector2f(190.f, 48.f));
    m_home.setCallback(&EndlessScreen::onHome, this);
    m_restart.init("Play Again", font, sf::Vector2f(340.f, 632.f), sf::Vector2f(190.f, 48.f));
    m_restart.setCallback(&EndlessScreen::onRestart, this);

    m_renderer.setCanvasRect(485.f, 135.f, 760.f, 500.f);
    m_session.start(TreeKind::BST, 73291, true);
    m_renderer.snapTo(m_session.snapshot());
    startRound();
}

bool EndlessScreen::containsKey(int key) const
{
    for (int i = 0; i < m_keys.size(); ++i)
    {
        if (m_keys[i] == key)
        {
            return true;
        }
    }
    return false;
}

void EndlessScreen::removeKey(int key)
{
    for (int i = 0; i < m_keys.size(); ++i)
    {
        if (m_keys[i] == key)
        {
            for (int j = i; j + 1 < m_keys.size(); ++j)
            {
                m_keys[j] = m_keys[j + 1];
            }
            m_keys.pop();
            return;
        }
    }
}

void EndlessScreen::startRound()
{
    m_random = m_random * 1664525u + 1013904223u;
    int operation = static_cast<int>((m_random >> 16) % 3u);
    if (m_keys.empty())
    {
        operation = SandboxOp::Insert;
    }
    m_expectedOperation = operation;
    if (operation == SandboxOp::Insert)
    {
        int candidate = 10 + static_cast<int>((m_random >> 5) % 90u);
        while (containsKey(candidate))
        {
            candidate = candidate == 99 ? 10 : candidate + 1;
        }
        m_expectedKey = candidate;
        m_prompt.setString("Insert a new key\nChoose the key yourself.");
    }
    else
    {
        m_random = m_random * 1664525u + 1013904223u;
        m_expectedKey = m_keys[static_cast<int>((m_random >> 16) % static_cast<unsigned int>(m_keys.size()))];
        const char* operationName = operation == SandboxOp::Search ? "Search" : "Delete";
        m_prompt.setString(std::string(operationName) + " for key\n" + std::to_string(m_expectedKey));
    }
    m_keyField.clear();
}

void EndlessScreen::applyAction(int operation)
{
    if (m_gameOver)
    {
        return;
    }
    if (!m_keyField.hasValue())
    {
        m_feedback = "Enter a key first.";
        return;
    }
    const int key = m_keyField.value();
    const bool correctOperation = operation == m_expectedOperation;
    const bool correctKey = operation == SandboxOp::Insert ? !containsKey(key) : key == m_expectedKey;
    if (!correctOperation || !correctKey)
    {
        const int penalty = mistakePenalty(m_score);
        m_seconds -= static_cast<float>(penalty);
        m_feedback = "Wrong move. " + std::to_string(penalty) + " seconds lost.";
        m_keyField.clear();
        if (m_seconds <= 0.f)
        {
            finishGame();
        }
        return;
    }

    SandboxResult result = m_session.apply(operation, key, 0);
    if (!result.accepted)
    {
        const int penalty = mistakePenalty(m_score);
        m_seconds -= static_cast<float>(penalty);
        m_feedback = result.message.empty() ? "Rejected. " + std::to_string(penalty) + " seconds lost." : wrapText(result.message, 36);
        m_keyField.clear();
        if (m_seconds <= 0.f)
        {
            finishGame();
        }
        return;
    }

    if (operation == SandboxOp::Insert)
    {
        m_keys.push(key);
    }
    else if (operation == SandboxOp::Delete)
    {
        removeKey(key);
    }
    m_score += 100;
    m_seconds += 2.f;
    if (m_seconds > 90.f)
    {
        m_seconds = 90.f;
    }
    m_feedback = "Correct move! +100 points, +2 seconds.";
    m_renderer.snapTo(m_session.snapshot());
    startRound();
}

void EndlessScreen::finishGame()
{
    if (m_gameOver)
    {
        return;
    }
    m_gameOver = true;
    m_seconds = 0.f;
    m_feedback = "Vault sealed. Final score: " + std::to_string(m_score);
    if (!m_scoreSaved)
    {
        std::error_code filesystemError;
        std::filesystem::create_directories("saves", filesystemError);
        Leaderboard leaderboard;
        leaderboard.load("saves/leaderboard.json");
        leaderboard.add("Player", m_score);
        leaderboard.save("saves/leaderboard.json");
        m_scoreSaved = true;
    }
}

void EndlessScreen::onAction(void* context)
{
    ActionContext* action = static_cast<ActionContext*>(context);
    action->owner->applyAction(action->operation);
}

void EndlessScreen::onHome(void* context)
{
    EndlessScreen* screen = static_cast<EndlessScreen*>(context);
    screen->m_ctx.ui->requestScreen(ScreenId::MainMenu);
}

void EndlessScreen::onRestart(void* context)
{
    EndlessScreen* screen = static_cast<EndlessScreen*>(context);
    screen->m_keys.clear();
    screen->m_seconds = 60.f;
    screen->m_score = 0;
    screen->m_gameOver = false;
    screen->m_scoreSaved = false;
    screen->m_feedback = "Round started. Good luck.";
    screen->m_session.start(TreeKind::BST, static_cast<int>(screen->m_random), true);
    screen->m_renderer.snapTo(screen->m_session.snapshot());
    screen->startRound();
}

void EndlessScreen::handleEvent(const sf::Event& event)
{
    if (!m_gameOver)
    {
        m_keyField.handleEvent(event);
        for (int i = 0; i < 3; ++i)
        {
            m_actions[i].handleEvent(event);
        }
    }
    m_home.handleEvent(event);
    if (m_gameOver)
    {
        m_restart.handleEvent(event);
    }
}

void EndlessScreen::update(float dt)
{
    if (!m_gameOver)
    {
        m_seconds -= dt;
        if (m_seconds <= 0.f)
        {
            finishGame();
        }
        m_renderer.update(dt);
        m_keyField.update(dt);
        for (int i = 0; i < 3; ++i)
        {
            m_actions[i].update(dt);
        }
    }
    m_home.update(dt);
    if (m_gameOver)
    {
        m_restart.update(dt);
    }
    const std::string scoreLabel = "SCORE  " + std::to_string(m_score);
    if (m_scoreText.getString().toAnsiString() != scoreLabel)
    {
        m_scoreText.setString(scoreLabel);
    }
    int seconds = static_cast<int>(m_seconds + 0.99f);
    if (seconds < 0)
    {
        seconds = 0;
    }
    const std::string timerLabel = "TIME  " + std::to_string(seconds) + "s";
    if (m_timerText.getString().toAnsiString() != timerLabel)
    {
        m_timerText.setString(timerLabel);
    }
    m_timerText.setFillColor(seconds <= 10 ? Theme::NodeGlow : Theme::TextColor);
    const std::string statusLabel = wrapText(m_feedback, 36);
    if (m_status.getString().toAnsiString() != statusLabel)
    {
        m_status.setString(statusLabel);
    }
    const std::string hint = m_gameOver ? "Your score is saved to the local leaderboard." : "Match the operation, enter its key, and keep an eye on the timer. Mistakes cost more as your score grows.";
    if (m_hint.getString().toAnsiString() != hint)
    {
        m_hint.setString(wrapText(hint, 43));
    }
}

void EndlessScreen::draw(sf::RenderWindow& window)
{
    window.draw(m_background);
    window.draw(m_dim);
    window.draw(m_panel);
    sf::RectangleShape headerRule(sf::Vector2f(382.f, 2.f));
    headerRule.setPosition(50.f, 137.f);
    headerRule.setFillColor(Theme::NodeLine);
    window.draw(headerRule);
    sf::RectangleShape timerTrack(sf::Vector2f(350.f, 9.f));
    timerTrack.setPosition(64.f, 177.f);
    timerTrack.setFillColor(sf::Color(65, 40, 45));
    window.draw(timerTrack);
    float timerProgress = m_seconds / 60.f;
    if (timerProgress < 0.f)
    {
        timerProgress = 0.f;
    }
    if (timerProgress > 1.f)
    {
        timerProgress = 1.f;
    }
    sf::RectangleShape timerFill(sf::Vector2f(350.f * timerProgress, 9.f));
    timerFill.setPosition(64.f, 177.f);
    timerFill.setFillColor(m_seconds <= 10.f ? Theme::NodeGlow : Theme::StarOn);
    window.draw(timerFill);
    window.draw(m_title);
    window.draw(m_scoreText);
    window.draw(m_timerText);
    window.draw(m_prompt);
    window.draw(m_status);
    window.draw(m_hint);
    if (!m_gameOver)
    {
        m_keyField.draw(window);
        for (int i = 0; i < 3; ++i)
        {
            m_actions[i].draw(window);
        }
    }
    else
    {
        m_restart.draw(window);
    }
    m_home.draw(window);
    m_renderer.draw(window);
}
