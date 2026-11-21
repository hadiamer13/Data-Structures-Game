#include "LevelLoader.h"

#include <fstream>
#include <sstream>

namespace
{
    struct NameValue
    {
        const char* name;
        int value;
    };

    const NameValue kTreeKinds[] =
        {
            {"binary", TreeKind::Binary}, {"bst", TreeKind::BST}, {"avl", TreeKind::AVL}, {"maxheap", TreeKind::MaxHeap}};
    const NameValue kMissions[] =
        {
            {"traversal", MissionKind::Traversal}, {"reconstruct", MissionKind::Reconstruct}, {"heightDiameter", MissionKind::HeightDiameter}, {"lcaPath", MissionKind::LcaPath}, {"mirror", MissionKind::Mirror}, {"bstSearch", MissionKind::BstSearch}, {"bstInsert", MissionKind::BstInsert}, {"bstDelete", MissionKind::BstDelete}, {"bstRange", MissionKind::BstRange}, {"bstBoss", MissionKind::BstBoss}, {"avlBadges", MissionKind::AvlBadges}, {"avlSingle", MissionKind::AvlSingle}, {"avlDouble", MissionKind::AvlDouble}, {"avlAuto", MissionKind::AvlAuto}, {"avlOrderStat", MissionKind::AvlOrderStat}, {"heapBuild", MissionKind::HeapBuild}, {"heapRepair", MissionKind::HeapRepair}, {"heapExtract", MissionKind::HeapExtract}, {"heapRace", MissionKind::HeapRace}, {"heapBoss", MissionKind::HeapBoss}, {"endless", MissionKind::Endless}};
    const NameValue kStyles[] =
        {
            {"random", 0}, {"skewed", 1}, {"sorted", 2}, {"reverse", 3}, {"heap", 4}, {"duplicates", 5}};

    bool readInteger(const JsonValue* object, const char* key, int& value, std::string* error)
    {
        const JsonValue* field = object == nullptr ? nullptr : object->get(key);
        if (field == nullptr || !field->asInteger(value))
        {
            if (error != nullptr)
            {
                *error = std::string("Missing or non-integer field: ") + key;
            }
            return false;
        }
        return true;
    }

    bool readString(const JsonValue* object, const char* key, std::string& value, std::string* error)
    {
        const JsonValue* field = object == nullptr ? nullptr : object->get(key);
        if (field == nullptr || !field->asString(value))
        {
            if (error != nullptr)
            {
                *error = std::string("Missing or non-string field: ") + key;
            }
            return false;
        }
        return true;
    }

    bool readNamedValue(const std::string& name, const NameValue* table, int count, int& value)
    {
        for (int i = 0; i < count; i++)
        {
            if (name == table[i].name)
            {
                value = table[i].value;
                return true;
            }
        }
        return false;
    }

    bool decodeLevel(const JsonValue* source, LevelDef& level, std::string* error)
    {
        std::string treeName;
        std::string missionName;
        std::string styleName;
        if (!readInteger(source, "index", level.index, error) ||
            !readString(source, "name", level.name, error) ||
            !readString(source, "concept", level.conceptText, error) ||
            !readString(source, "objective", level.objective, error) ||
            !readString(source, "treeKind", treeName, error) ||
            !readString(source, "mission", missionName, error) ||
            !readInteger(source, "seed", level.seed, error) ||
            !readInteger(source, "nodeCount", level.nodeCount, error) ||
            !readString(source, "style", styleName, error) ||
            !readInteger(source, "param", level.param, error))
        {
            return false;
        }
        if (!readNamedValue(treeName, kTreeKinds, 4, level.treeKind) ||
            !readNamedValue(missionName, kMissions, 21, level.mission) ||
            !readNamedValue(styleName, kStyles, 6, level.style))
        {
            if (error != nullptr)
            {
                *error = "Unknown treeKind, mission, or style value";
            }
            return false;
        }
        const JsonValue* boss = source->get("boss");
        if (boss == nullptr || !boss->asBoolean(level.boss))
        {
            if (error != nullptr)
            {
                *error = "Missing or non-boolean field: boss";
            }
            return false;
        }
        const JsonValue* preorder = source->get("preorder");
        if (preorder != nullptr && !preorder->asString(level.preorder))
        {
            if (error != nullptr)
            {
                *error = "Field preorder must be a string";
            }
            return false;
        }
        if (level.index < 1 || level.index > 5 || level.seed < 0 || level.nodeCount < 0)
        {
            if (error != nullptr)
            {
                *error = "Level index, seed, or nodeCount is out of range";
            }
            return false;
        }
        return true;
    }
}

LevelDefArray::LevelDefArray() : m_data(nullptr), m_size(0), m_capacity(0)
{
}

LevelDefArray::LevelDefArray(const LevelDefArray& other) : m_data(nullptr), m_size(0), m_capacity(0)
{
    *this = other;
}

LevelDefArray& LevelDefArray::operator=(const LevelDefArray& other)
{
    if (this != &other)
    {
        clear();
        reserve(other.m_size);
        for (int i = 0; i < other.m_size; i++)
        {
            m_data[i] = other.m_data[i];
        }
        m_size = other.m_size;
    }
    return *this;
}

LevelDefArray::~LevelDefArray()
{
    delete[] m_data;
}

int LevelDefArray::size() const
{
    return m_size;
}

bool LevelDefArray::empty() const
{
    return m_size == 0;
}

LevelDef& LevelDefArray::operator[](int index)
{
    return m_data[index];
}

const LevelDef& LevelDefArray::operator[](int index) const
{
    return m_data[index];
}

void LevelDefArray::reserve(int capacity)
{
    if (capacity <= m_capacity)
    {
        return;
    }
    LevelDef* data = new LevelDef[capacity];
    for (int i = 0; i < m_size; i++)
    {
        data[i] = m_data[i];
    }
    delete[] m_data;
    m_data = data;
    m_capacity = capacity;
}

void LevelDefArray::push(const LevelDef& value)
{
    if (m_size == m_capacity)
    {
        reserve(m_capacity == 0 ? 4 : m_capacity * 2);
    }
    m_data[m_size++] = value;
}

void LevelDefArray::clear()
{
    m_size = 0;
}

bool LevelLoader::missionFromName(const std::string& name, int& mission)
{
    return readNamedValue(name, kMissions, 21, mission);
}

bool LevelLoader::loadFile(const std::string& path, LevelDefArray& out, std::string* error)
{
    std::ifstream input(path.c_str(), std::ios::binary);
    if (!input)
    {
        if (error != nullptr)
        {
            *error = std::string("Unable to open level file: ") + path;
        }
        return false;
    }
    std::ostringstream contents;
    contents << input.rdbuf();
    return loadJson(contents.str(), out, error);
}

bool LevelLoader::loadJson(const std::string& text, LevelDefArray& out, std::string* error)
{
    if (error != nullptr)
    {
        error->clear();
    }
    JsonDocument document;
    JsonError parseError;
    if (!JsonParser::parse(text, document, &parseError))
    {
        if (error != nullptr)
        {
            std::ostringstream message;
            message << parseError.message << " at character " << parseError.position;
            *error = message.str();
        }
        return false;
    }
    int act = 0;
    const JsonValue* root = document.root();
    if (root == nullptr || root->type() != JsonType::Object || !readInteger(root, "act", act, error) || act < 1 || act > 4)
    {
        if (error != nullptr && error->empty())
        {
            *error = "Root must contain an act number from 1 to 4";
        }
        return false;
    }
    const JsonValue* levels = root->get("levels");
    if (levels == nullptr || levels->type() != JsonType::Array || levels->size() != 5)
    {
        if (error != nullptr)
        {
            *error = "The act must contain exactly five level definitions";
        }
        return false;
    }
    LevelDefArray loaded;
    for (int i = 0; i < levels->size(); i++)
    {
        LevelDef level;
        level.act = act;
        if (!decodeLevel(levels->at(i), level, error) || level.index != i + 1)
        {
            if (error != nullptr && error->empty())
            {
                *error = "Level indices must be in order from 1 to 5";
            }
            return false;
        }
        loaded.push(level);
    }
    out = loaded;
    return true;
}
