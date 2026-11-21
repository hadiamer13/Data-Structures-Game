#pragma once

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

    bool load();
    bool loaded() const;
    const std::string& error() const;

    const LevelDef& level(int index) const;
    int stars(int index) const;
    bool isBoss(int index) const;

    bool isUnlocked(int index) const;

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
