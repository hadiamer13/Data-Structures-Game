#ifndef VH_LEADERBOARD_H
#define VH_LEADERBOARD_H

#include <string>

struct ScoreEntry
{
    std::string name;
    int score;

    ScoreEntry() : score(0)
    {
    }
};

// Fixed-size local top-ten score table. Higher scores rank first; ties use
// player name order for repeatable results.
class Leaderboard
{
public:
    static const int Capacity = 10;

    Leaderboard();
    int size() const;
    const ScoreEntry& at(int index) const;
    bool add(const std::string& name, int score);
    void clear();
    bool load(const std::string& path, std::string* error = nullptr);
    bool save(const std::string& path, std::string* error = nullptr) const;

private:
    ScoreEntry m_entries[Capacity];
    int m_size;
};

#endif
