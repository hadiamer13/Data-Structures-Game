#ifndef VH_LEVELLOADER_H
#define VH_LEVELLOADER_H

#include "Json.h"
#include "../contracts/GameTypes.h"

class LevelDefArray
{
public:
    LevelDefArray();
    LevelDefArray(const LevelDefArray& other);
    LevelDefArray& operator=(const LevelDefArray& other);
    ~LevelDefArray();

    int size() const;
    bool empty() const;
    LevelDef& operator[](int index);
    const LevelDef& operator[](int index) const;
    void push(const LevelDef& value);
    void clear();

private:
    void reserve(int capacity);
    LevelDef* m_data;
    int m_size;
    int m_capacity;
};

class LevelLoader
{
public:
    static bool loadFile(const std::string& path, LevelDefArray& out, std::string* error = nullptr);
    static bool loadJson(const std::string& text, LevelDefArray& out, std::string* error = nullptr);
    static bool missionFromName(const std::string& name, int& mission);
};

#endif
