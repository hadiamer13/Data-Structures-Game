#include "ScoreSystem.h"

ScoreSystem::ScoreSystem()
    : m_alarm(0), m_alarmCap(2), m_stepsTaken(0), m_optimalSteps(1),
    m_penalties(0), m_penaltyCount(0), m_status(HudStatus::Playing), m_hintUsed(false)
{
}

void ScoreSystem::configure(int optimalSteps, const std::string& objective)
{
    m_optimalSteps = optimalSteps < 1 ? 1 : optimalSteps;
    m_alarmCap = (3 * m_optimalSteps + 1) / 2 + 2;
    m_objective = objective;
    reset();
}

void ScoreSystem::reset()
{
    m_alarm = 0;
    m_stepsTaken = 0;
    m_penalties = 0;
    m_penaltyCount = 0;
    m_status = HudStatus::Playing;
    m_hintUsed = false;
}

void ScoreSystem::recordAction(int alarmCost, int penalty)
{
    if (m_status != HudStatus::Playing)
    {
        return;
    }
    m_stepsTaken = m_stepsTaken + 1;
    m_alarm = m_alarm + (alarmCost < 0 ? 0 : alarmCost);
    m_penalties = m_penalties + (penalty < 0 ? 0 : penalty);
    if (penalty > 0)
    {
        m_penaltyCount = m_penaltyCount + 1;
    }
    if (m_alarm >= m_alarmCap)
    {
        m_alarm = m_alarmCap;
        m_status = HudStatus::Lost;
    }
}

void ScoreSystem::markWon()
{
    if (m_status == HudStatus::Playing)
    {
        m_status = HudStatus::Won;
    }
}

void ScoreSystem::markLost()
{
    m_status = HudStatus::Lost;
}

int ScoreSystem::alarm() const { return m_alarm; }
int ScoreSystem::alarmCap() const { return m_alarmCap; }
int ScoreSystem::stepsTaken() const { return m_stepsTaken; }
int ScoreSystem::optimalSteps() const { return m_optimalSteps; }
int ScoreSystem::penalties() const { return m_penalties; }
int ScoreSystem::status() const { return m_status; }

int ScoreSystem::score() const
{
    const int value = 1000 - 10 * m_alarm - m_penalties;
    return value < 0 ? 0 : value;
}

void ScoreSystem::useHint()
{
    m_hintUsed = true;
}

bool ScoreSystem::hintUsed() const
{
    return m_hintUsed;
}

int ScoreSystem::stars() const
{
    if (m_status != HudStatus::Won)
    {
        return 0;
    }
    int result = 1;
    if (m_stepsTaken <= m_optimalSteps && m_penalties == 0)
    {
        result = 3;
    }
    else if (4 * m_stepsTaken <= 5 * m_optimalSteps && m_penaltyCount <= 1)
    {
        result = 2;
    }
    if (m_hintUsed && result > 2)
    {
        result = 2;     // hint = star cap of 2
    }
    return result;
}

HudState ScoreSystem::hud() const
{
    HudState state;
    state.alarm = m_alarm;
    state.alarmCap = m_alarmCap;
    state.score = score();
    state.stepsTaken = m_stepsTaken;
    state.optimalSteps = m_optimalSteps;
    state.penalties = m_penalties;
    state.stars = stars();
    state.status = m_status;
    state.objective = m_objective;
    return state;
}
