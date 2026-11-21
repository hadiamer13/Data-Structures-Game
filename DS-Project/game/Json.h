#ifndef VH_JSON_H
#define VH_JSON_H

#include <string>
#include "../contracts/Containers.h"

class JsonValue;

namespace JsonType
{
    inline const int Null = 0;
    inline const int Boolean = 1;
    inline const int Integer = 2;
    inline const int String = 3;
    inline const int Array = 4;
    inline const int Object = 5;
}

class JsonPtrArray
{
  public:
    JsonPtrArray();
    JsonPtrArray(const JsonPtrArray&) = delete;
    JsonPtrArray& operator=(const JsonPtrArray&) = delete;
    ~JsonPtrArray();

    int size() const;
    JsonValue*& operator[](int index);
    JsonValue* const& operator[](int index) const;
    void push(JsonValue* value);
    void clear();

  private:
    void reserve(int capacity);
    JsonValue** m_data;
    int m_size;
    int m_capacity;
};

class JsonValue
{
  public:
    explicit JsonValue(int type = JsonType::Null);
    JsonValue(const JsonValue&) = delete;
    JsonValue& operator=(const JsonValue&) = delete;
    ~JsonValue();

    int type() const;
    bool asBoolean(bool& value) const;
    bool asInteger(int& value) const;
    bool asString(std::string& value) const;
    int size() const;
    const JsonValue* at(int index) const;
    const JsonValue* get(const std::string& key) const;
    const std::string& keyAt(int index) const;

  private:
    friend class JsonParser;
    friend class JsonParserState;
    int m_type;
    bool m_boolean;
    int m_integer;
    std::string m_string;
    StringArray m_keys;
    JsonPtrArray m_children;
};

struct JsonError
{
    int position;
    std::string message;

    JsonError() : position(0)
    {
    }
};

class JsonDocument
{
  public:
    JsonDocument();
    JsonDocument(const JsonDocument&) = delete;
    JsonDocument& operator=(const JsonDocument&) = delete;
    ~JsonDocument();

    const JsonValue* root() const;
    void clear();

  private:
    friend class JsonParser;
    JsonValue* m_root;
};

class JsonParser
{
  public:
    static bool parse(const std::string& text, JsonDocument& document, JsonError* error = nullptr);
};

class JsonWriter
{
  public:
    static std::string quote(const std::string& value);
};

#endif
