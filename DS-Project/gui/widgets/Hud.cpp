// gui/widgets/Hud.cpp
#include "Hud.h"
#include "../Theme.h"

namespace
{
    float hMin(float a, float b)
    {
        return (a < b) ? a : b;
    }

    float hMax(float a, float b)
    {
        return (a > b) ? a : b;
    }

    const float Pad = 16.f;
    const float ObjectiveWrapW = 560.f;
    const float BarH = 20.f;
}

Hud::Hud()
    : m_x(0.f), m_y(0.f), m_w(1280.f), m_h(84.f),
    m_alarm(0), m_cap(1), m_score(0), m_steps(0), m_maxSteps(0), m_shown(0.f)
{
}

void Hud::init(const sf::Font& font, float x, float y, float w, float h)
{
    m_x = x;
    m_y = y;
    m_w = w;
    m_h = h;

    m_bg.setPosition(x, y);
    m_bg.setSize(sf::Vector2f(w, h));
    m_bg.setFillColor(Theme::TooltipBg);
    m_bg.setOutlineColor(Theme::BtnOutline);
    m_bg.setOutlineThickness(-2.f);

    m_title.setFont(font);
    m_title.setCharacterSize(24);
    m_title.setFillColor(Theme::TitleColor);
    m_title.setPosition(x + Pad, y + 6.f);

    m_objective.setFont(font);
    m_objective.setCharacterSize(14);
    m_objective.setFillColor(Theme::TextColor);
    m_objective.setPosition(x + Pad, y + 38.f);

    // alarm meter (middle)
    float mx = x + w * 0.47f;
    float mw = 300.f;
    m_alarmLabel.setFont(font);
    m_alarmLabel.setCharacterSize(16);
    m_alarmLabel.setFillColor(Theme::TextColor);
    m_alarmLabel.setPosition(mx, y + 12.f);

    m_barBack.setPosition(mx, y + 46.f);
    m_barBack.setSize(sf::Vector2f(mw, BarH));
    m_barBack.setFillColor(sf::Color(30, 22, 26));
    m_barBack.setOutlineColor(Theme::NodeLine);
    m_barBack.setOutlineThickness(2.f);

    m_barFill.setPosition(mx, y + 46.f);
    m_barFill.setSize(sf::Vector2f(0.f, BarH));

    for (int i = 0; i < 3; ++i)
    {
        m_tick[i].setSize(sf::Vector2f(2.f, BarH));
        m_tick[i].setPosition(mx + mw * 0.25f * (float)(i + 1) - 1.f, y + 46.f);
        m_tick[i].setFillColor(sf::Color(200, 190, 190, 150));
    }

    // right block
    float rx = x + w - 290.f;
    m_scoreLabel.setFont(font);
    m_scoreLabel.setCharacterSize(18);
    m_scoreLabel.setFillColor(Theme::StarOn);
    m_scoreLabel.setPosition(rx, y + 10.f);

    m_stepsLabel.setFont(font);
    m_stepsLabel.setCharacterSize(16);
    m_stepsLabel.setFillColor(Theme::TextColor);
    m_stepsLabel.setPosition(rx, y + 38.f);

    m_status.setFont(font);
    m_status.setCharacterSize(16);
    m_status.setFillColor(Theme::NodeGlow);
    m_status.setPosition(rx + 170.f, y + 10.f);

    setScore(0);
    setSteps(0, 0);
    setStatus("READY");
    refreshMeterLabel();
}

void Hud::setTitle(const std::string& title)
{
    m_title.setString(title);
}

void Hud::setObjective(const std::string& objective)
{
    m_objectiveRaw = objective;
    layoutObjective();
}

void Hud::layoutObjective()
{
    // '\n' -> space, greedy word wrap to ObjectiveWrapW using the real font metrics.
    std::string flat;
    for (size_t i = 0; i < m_objectiveRaw.size(); ++i)
    {
        flat += (m_objectiveRaw[i] == '\n') ? ' ' : m_objectiveRaw[i];
    }

    std::string result;
    std::string line;
    std::string word;
    size_t i = 0;
    while (i <= flat.size())
    {
        char c = (i < flat.size()) ? flat[i] : ' ';
        if (c != ' ')
        {
            word += c;
        }
        else if (!word.empty())
        {
            std::string candidate = line.empty() ? word : (line + " " + word);
            m_objective.setString(candidate);
            if (m_objective.getLocalBounds().width > ObjectiveWrapW && !line.empty())
            {
                result += line + "\n";
                line = word;
            }
            else
            {
                line = candidate;
            }
            word.clear();
        }
        ++i;
    }
    result += line;
    m_objective.setString(result);
}

void Hud::refreshMeterLabel()
{
    std::string s = "ALARM " + std::to_string(m_alarm) + " / " + std::to_string(m_cap);
    float ratio = (m_cap > 0) ? (float)m_alarm / (float)m_cap : 0.f;
    if (m_alarm >= m_cap && m_cap > 0)
    {
        s += "  - FULL";
    }
    else if (ratio >= 0.85f)
    {
        s += "  - DANGER";
    }
    m_alarmLabel.setString(s);
}

void Hud::setAlarm(int alarm, int capacity)
{
    if (alarm == m_alarm && capacity == m_cap)
    {
        return;
    }
    m_alarm = alarm;
    m_cap = (capacity > 0) ? capacity : 1;
    refreshMeterLabel();
}

void Hud::setScore(int score)
{
    m_score = score;
    m_scoreLabel.setString("SCORE  " + std::to_string(score));
}

void Hud::setSteps(int steps, int maxSteps)
{
    m_steps = steps;
    m_maxSteps = maxSteps;
    m_stepsLabel.setString("STEPS  " + std::to_string(steps) + " / " + std::to_string(maxSteps));
}

void Hud::setStatus(const std::string& status)
{
    m_status.setString(status);
}

void Hud::update(float dt)
{
    float target = (m_cap > 0) ? hMin(1.f, (float)m_alarm / (float)m_cap) : 0.f;
    float step = 2.5f * dt;
    if (m_shown < target)
    {
        m_shown = hMin(target, m_shown + step);
    }
    else
    {
        m_shown = hMax(target, m_shown - step);
    }

    sf::Color col(60, 160, 80);
    if (m_shown >= 0.85f)
    {
        col = sf::Color(210, 50, 50);
    }
    else if (m_shown >= 0.6f)
    {
        col = sf::Color(215, 180, 50);
    }
    m_barFill.setFillColor(col);
    m_barFill.setSize(sf::Vector2f(m_barBack.getSize().x * m_shown, BarH));
}

void Hud::draw(sf::RenderWindow& window) const
{
    window.draw(m_bg);
    window.draw(m_title);
    window.draw(m_objective);

    window.draw(m_alarmLabel);
    window.draw(m_barBack);
    window.draw(m_barFill);
    for (int i = 0; i < 3; ++i)
    {
        window.draw(m_tick[i]);
    }

    window.draw(m_scoreLabel);
    window.draw(m_stepsLabel);
    window.draw(m_status);
}
