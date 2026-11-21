#include "Leaderboard.h"

#include <fstream>
#include <sstream>

#include "Json.h"

Leaderboard::Leaderboard() : m_size(0)
{
}

int Leaderboard::size() const
{
    return m_size;
}

const ScoreEntry& Leaderboard::at(int index) const
{
    return m_entries[index];
}

bool Leaderboard::add(const std::string& name, int score)
{
    if (name.empty() || score < 0)
    {
        return false;
    }

    for (int i = 0; i < static_cast<int>(name.size()); i++)
    {
        if (static_cast<unsigned char>(name[i]) < 0x20)
        {
            return false;
        }
    }

    ScoreEntry candidate;
    candidate.name = name;
    candidate.score = score;
    int position = 0;

    while (position < m_size && (m_entries[position].score > score ||
                                 (m_entries[position].score == score && m_entries[position].name <= name)))
    {
        position++;
    }

    if (position >= Capacity)
    {
        return false;
    }

    const int newSize = m_size < Capacity ? m_size + 1 : m_size;

    for (int i = newSize - 1; i > position; i--)
    {
        m_entries[i] = m_entries[i - 1];
    }

    m_entries[position] = candidate;
    m_size = newSize;
    return true;
}

void Leaderboard::clear()
{
    m_size = 0;
}

bool Leaderboard::load(const std::string& path, std::string* error)
{
    if (error != nullptr)
    {
        error->clear();
    }

    std::ifstream file(path.c_str(), std::ios::binary);

    if (!file)
    {
        if (error != nullptr)
        {
            *error = std::string("Unable to open leaderboard: ") + path;
        }

        return false;
    }

    std::ostringstream contents;
    contents << file.rdbuf();
    JsonDocument document;
    JsonError parseError;

    if (!JsonParser::parse(contents.str(), document, &parseError))
    {
        if (error != nullptr)
        {
            std::ostringstream message;
            message << parseError.message << " at character " << parseError.position;
            *error = message.str();
        }

        return false;
    }

    const JsonValue* scores = document.root() == nullptr ? nullptr : document.root()->get("scores");

    if (scores == nullptr || scores->type() != JsonType::Array)
    {
        if (error != nullptr)
        {
            *error = "Leaderboard must contain a scores array";
        }

        return false;
    }

    Leaderboard loaded;

    for (int i = 0; i < scores->size(); i++)
    {
        const JsonValue* row = scores->at(i);
        const JsonValue* nameValue = row == nullptr ? nullptr : row->get("name");
        const JsonValue* scoreValue = row == nullptr ? nullptr : row->get("score");
        std::string name;

        int score = 0;

        if (nameValue == nullptr || !nameValue->asString(name) || scoreValue == nullptr || !scoreValue->asInteger(score) ||
            !loaded.add(name, score))
        {
            if (error != nullptr)
            {
                *error = "Leaderboard contains an invalid score entry";
            }

            return false;
        }
    }

    *this = loaded;
    return true;
}

bool Leaderboard::save(const std::string& path, std::string* error) const
{
    if (error != nullptr)
    {
        error->clear();
    }

    std::ostringstream contents;
    contents << "{\n  \"scores\": [";

    for (int i = 0; i < m_size; i++)
    {
        if (i > 0)
        {
            contents << ',';
        }

        contents << "\n    {\"name\": " << JsonWriter::quote(m_entries[i].name)
                 << ", \"score\": " << m_entries[i].score << '}';
    }

    if (m_size > 0)
    {
        contents << '\n';
    }

    contents << "  ]\n}\n";

    std::ofstream file(path.c_str(), std::ios::binary | std::ios::trunc);

    if (!file)
    {
        if (error != nullptr)
        {
            *error = std::string("Unable to open leaderboard for writing: ") + path;
        }

        return false;
    }

    file << contents.str();

    if (!file)
    {
        if (error != nullptr)
        {
            *error = std::string("Failed while writing leaderboard: ") + path;
        }

        return false;
    }

    return true;
}
