#ifndef VH_SCORESYSTEM_H
#define VH_SCORESYSTEM_H

#include <string>
#include "../contracts/GameTypes.h"

// Pure C++ alarm, score, penalty and star rules; it has no GUI dependencies.
class ScoreSystem
{
public:
    ScoreSystem();

    void configure(int optimalSteps, const std::string& objective);
    void reset();
    void recordAction(int alarmCost, int penalty = 0);
    void markWon();
    void markLost();

    int alarm() const;
    int alarmCap() const;
    int score() const;
    int stepsTaken() const;
    int optimalSteps() const;
    int penalties() const;
    int stars() const;
    int status() const;
    HudState hud() const;

    void useHint();
    bool hintUsed() const;

private:
    int m_alarm;
    int m_alarmCap;
    bool m_hintUsed;
    int m_stepsTaken;
    int m_optimalSteps;
    int m_penalties;
    int m_penaltyCount;
    int m_status;
    std::string m_objective;
};

#endif
