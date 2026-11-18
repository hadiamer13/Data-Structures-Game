#pragma once
// game/Campaign.h
// Shared campaign state: 20 level definitions (levels/act1.json .. act4.json) plus
// star progress. Index = act * 5 + level (0..19), the same index as AppContext::selectedLevel.
// Pure C++, no SFML.


#include <string>
#include "LevelLoader.h"
#include "SaveManager.h"

class Campaign
{
public:
    static const int ActCount = 4;
    static const int PerAct = 5;
    static const int LevelCount = 20;

    Campaign();

    // Reads all four act files. Returns true only if every file loaded.
    bool load();
    bool loaded() const;
    const std::string& error() const;

    const LevelDef& level(int index) const;
    int stars(int index) const;
    bool isBoss(int index) const;

    // Level 0 (1.1) is always open. Any other level needs at least 1 star on the
    // previous level, unless the debug unlock-all flag is on.
    bool isUnlocked(int index) const;

    // Keeps the best value: replaying a level never lowers its stars.
    void setStars(int index, int stars);

    bool unlockAll() const;
    void toggleUnlockAll();

private:
    bool validIndex(int index) const;

    LevelDefArray m_acts[ActCount];
    SaveData m_save;
    LevelDef m_empty;
    std::string m_error;
    bool m_loaded;
    bool m_unlockAll;
};
