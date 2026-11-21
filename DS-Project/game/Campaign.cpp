// game/Campaign.cpp
#include "Campaign.h"

Campaign::Campaign() : m_loaded(false), m_unlockAll(false)
{
}

bool Campaign::load()
{
    m_error.clear();
    m_loaded = true;

    for (int act = 0; act < ActCount; act++)
    {
        const std::string path = std::string("levels/act") + static_cast<char>('1' + act) + ".json";

        std::string error;

        if (!LevelLoader::loadFile(path, m_acts[act], &error))
        {
            m_loaded = false;

            if (!m_error.empty())
            {
                m_error += "\n";
            }

            m_error += path + ": " + error;
        }
    }
    return m_loaded;
}

bool Campaign::loaded() const
{
    return m_loaded;
}

const std::string& Campaign::error() const
{
    return m_error;
}

bool Campaign::validIndex(int index) const
{
    return index >= 0 && index < LevelCount;
}

const LevelDef& Campaign::level(int index) const
{
    if (!validIndex(index))
    {
        return m_empty;
    }

    const int act = index / PerAct;

    if (m_acts[act].size() != PerAct)
    {
        return m_empty;
    }

    return m_acts[act][index % PerAct];
}

int Campaign::stars(int index) const
{
    if (!validIndex(index))
    {
        return 0;
    }

    return m_save.stars[index];
}

bool Campaign::isBoss(int index) const
{
    return level(index).boss;
}

bool Campaign::isUnlocked(int index) const
{
    if (!validIndex(index) || !m_loaded)
    {
        return false;
    }

    if (m_unlockAll || index == 0)
    {
        return true;
    }

    return m_save.stars[index - 1] >= 1;
}

void Campaign::setStars(int index, int stars)
{
    if (!validIndex(index))
    {
        return;
    }

    int clamped = stars;

    if (clamped < 0)
    {
        clamped = 0;
    }

    if (clamped > 3)
    {
        clamped = 3;
    }

    if (clamped > m_save.stars[index])
    {
        m_save.stars[index] = clamped;
    }
}

bool Campaign::unlockAll() const
{
    return m_unlockAll;
}

void Campaign::toggleUnlockAll()
{
    m_unlockAll = !m_unlockAll;
}