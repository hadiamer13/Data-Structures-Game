#ifndef VH_SAVEMANAGER_H
#define VH_SAVEMANAGER_H

#include <string>
#include "../contracts/GameTypes.h"

struct SaveData
{
    IntArray stars;
    int endlessSeed;
    int endlessTreeKind;
    std::string endlessTree;
    IntArray endlessHeap;
    int activeSize;
    int bestScore;

    SaveData();
};

class SaveManager
{
  public:
    static bool save(const std::string& path, const SaveData& data, std::string* error = nullptr);
    static bool load(const std::string& path, SaveData& out, std::string* error = nullptr);
    static bool validate(const SaveData& data, std::string* error = nullptr);
};

#endif
