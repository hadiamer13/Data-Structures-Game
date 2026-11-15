// gui/widgets/Hud.h
// Top bar: level title + objective (left), alarm meter (middle),
// score / steps / status (right).
// Knows nothing about the model. The owner feeds it numbers
// (INTEGRATION: from ActionResult.hud, never from GUI maths).
// Colour is never the only signal: the meter always has a numeric label and
// a DANGER word near the limit; the status is a word.
#pragma once
#include <string>
#include <SFML/Graphics.hpp>

class Hud
{
public:
    Hud();

    void init(const sf::Font& font, float x, float y, float w, float h);

    void setTitle(const std::string& title);
    void setObjective(const std::string& objective);   // '\n' becomes a space, then word-wrapped
    void setAlarm(int alarm, int capacity);
    void setScore(int score);
    void setSteps(int steps, int maxSteps);
    void setStatus(const std::string& status);

    void update(float dt);
    void draw(sf::RenderWindow& window) const;

private:
    void layoutObjective();
    void refreshMeterLabel();

    float m_x;
    float m_y;
    float m_w;
    float m_h;

    std::string m_objectiveRaw;

    int m_alarm;
    int m_cap;
    int m_score;
    int m_steps;
    int m_maxSteps;
    float m_shown;     // eased meter fill 0..1

    sf::RectangleShape m_bg;
    sf::RectangleShape m_barBack;
    sf::RectangleShape m_barFill;
    sf::RectangleShape m_tick[3];

    sf::Text m_title;
    sf::Text m_objective;
    sf::Text m_alarmLabel;
    sf::Text m_scoreLabel;
    sf::Text m_stepsLabel;
    sf::Text m_status;
};
