#include "Json.h"

#include <climits>
#include <sstream>

JsonPtrArray::JsonPtrArray() : m_data(nullptr), m_size(0), m_capacity(0)
{
}

JsonPtrArray::~JsonPtrArray()
{
    delete[] m_data;
}

int JsonPtrArray::size() const
{
    return m_size;
}

JsonValue*& JsonPtrArray::operator[](int index)
{
    return m_data[index];
}

JsonValue* const& JsonPtrArray::operator[](int index) const
{
    return m_data[index];
}

void JsonPtrArray::reserve(int capacity)
{
    if (capacity <= m_capacity)
    {
        return;
    }

    JsonValue** data = new JsonValue*[capacity];

    for (int i = 0; i < m_size; i++)
    {
        data[i] = m_data[i];
    }

    delete[] m_data;
    m_data = data;
    m_capacity = capacity;
}

void JsonPtrArray::push(JsonValue* value)
{
    if (m_size == m_capacity)
    {
        reserve(m_capacity == 0 ? 4 : m_capacity * 2);
    }

    m_data[m_size++] = value;
}

void JsonPtrArray::clear()
{
    m_size = 0;
}

JsonValue::JsonValue(int type) : m_type(type), m_boolean(false), m_integer(0)
{
}

JsonValue::~JsonValue()
{
    for (int i = 0; i < m_children.size(); i++)
    {
        delete m_children[i];
    }
}

int JsonValue::type() const
{
    return m_type;
}

bool JsonValue::asBoolean(bool& value) const
{
    if (m_type != JsonType::Boolean)
    {
        return false;
    }

    value = m_boolean;
    return true;
}

bool JsonValue::asInteger(int& value) const
{
    if (m_type != JsonType::Integer)
    {
        return false;
    }

    value = m_integer;
    return true;
}

bool JsonValue::asString(std::string& value) const
{
    if (m_type != JsonType::String)
    {
        return false;
    }

    value = m_string;
    return true;
}

int JsonValue::size() const
{
    return m_children.size();
}

const JsonValue* JsonValue::at(int index) const
{
    return index < 0 || index >= m_children.size() ? nullptr : m_children[index];
}

const JsonValue* JsonValue::get(const std::string& key) const
{
    if (m_type != JsonType::Object)
    {
        return nullptr;
    }

    for (int i = 0; i < m_keys.size(); i++)
    {
        if (m_keys[i] == key)
        {
            return m_children[i];
        }
    }
    return nullptr;
}

const std::string& JsonValue::keyAt(int index) const
{
    return m_keys[index];
}

JsonDocument::JsonDocument() : m_root(nullptr)
{
}

JsonDocument::~JsonDocument()
{
    clear();
}

const JsonValue* JsonDocument::root() const
{
    return m_root;
}

void JsonDocument::clear()
{
    delete m_root;
    m_root = nullptr;
}

class JsonParserState
{
  public:
    JsonParserState(const std::string& text, JsonError* error) : source(text), position(0), errorOut(error), failed(false)
    {
        if (errorOut != nullptr)
        {
            errorOut->position = 0;
            errorOut->message.clear();
        }
    }

    JsonValue* parseValue(int depth)
    {
        if (depth > 256)
        {
            return fail("Maximum nesting depth exceeded");
        }

        skipWhitespace();

        if (position >= source.size())
        {
            return fail("Expected a value");
        }

        const char c = source[position];

        if (c == '{')
        {
            return parseObject(depth + 1);
        }

        if (c == '[')
        {
            return parseArray(depth + 1);
        }

        if (c == '"')
        {
            JsonValue* value = new JsonValue(JsonType::String);

            if (!parseString(value->m_string))
            {
                delete value;
                return nullptr;
            }

            return value;
        }

        if (c == 't' && consumeLiteral("true"))
        {
            JsonValue* value = new JsonValue(JsonType::Boolean);
            value->m_boolean = true;
            return value;
        }

        if (c == 'f' && consumeLiteral("false"))
        {
            return new JsonValue(JsonType::Boolean);
        }

        if (c == 'n' && consumeLiteral("null"))
        {
            return new JsonValue(JsonType::Null);
        }

        if (c == '-' || (c >= '0' && c <= '9'))
        {
            return parseInteger();
        }

        return fail("Unexpected character while parsing a value");
    }

    const std::string& source;
    int position;
    JsonError* errorOut;
    bool failed;

    void skipWhitespace()
    {
        while (position < source.size() && (source[position] == ' ' || source[position] == '\t' ||
                                            source[position] == '\r' || source[position] == '\n'))
        {
            position++;
        }
    }

    JsonValue* fail(const char* message)
    {
        if (!failed && errorOut != nullptr)
        {
            errorOut->position = position;
            errorOut->message = message;
        }

        failed = true;
        return nullptr;
    }

    JsonValue* parseObject(int depth)
    {
        position++;
        JsonValue* object = new JsonValue(JsonType::Object);
        skipWhitespace();

        if (position < source.size() && source[position] == '}')
        {
            position++;
            return object;
        }

        while (!failed)
        {
            skipWhitespace();

            if (position >= source.size() || source[position] != '"')
            {
                delete object;
                return fail("Expected an object key");
            }

            std::string key;

            if (!parseString(key))
            {
                delete object;
                return nullptr;
            }

            if (object->get(key) != nullptr)
            {
                delete object;
                return fail("Duplicate object key");
            }

            skipWhitespace();

            if (position >= source.size() || source[position] != ':')
            {
                delete object;
                return fail("Expected ':' after object key");
            }

            position++;
            JsonValue* child = parseValue(depth);

            if (child == nullptr)
            {
                delete object;
                return nullptr;
            }

            object->m_keys.push(key);
            object->m_children.push(child);
            skipWhitespace();

            if (position < source.size() && source[position] == '}')
            {
                position++;
                return object;
            }

            if (position >= source.size() || source[position] != ',')
            {
                delete object;
                return fail("Expected ',' or '}' in object");
            }

            position++;
        }

        delete object;
        return nullptr;
    }

    JsonValue* parseArray(int depth)
    {
        position++;
        JsonValue* array = new JsonValue(JsonType::Array);
        skipWhitespace();

        if (position < source.size() && source[position] == ']')
        {
            position++;
            return array;
        }

        while (!failed)
        {
            JsonValue* child = parseValue(depth);

            if (child == nullptr)
            {
                delete array;
                return nullptr;
            }

            array->m_children.push(child);
            skipWhitespace();

            if (position < source.size() && source[position] == ']')
            {
                position++;
                return array;
            }

            if (position >= source.size() || source[position] != ',')
            {
                delete array;
                return fail("Expected ',' or ']' in array");
            }

            position++;
        }

        delete array;
        return nullptr;
    }

    bool parseString(std::string& out)
    {
        if (source[position] != '"')
        {
            return false;
        }

        position++;
        out.clear();

        while (position < source.size())
        {
            const unsigned char c = static_cast<unsigned char>(source[position++]);

            if (c == '"')
            {
                return true;
            }

            if (c < 0x20)
            {
                fail("Unescaped control character in string");
                return false;
            }

            if (c != '\\')
            {
                out.push_back(static_cast<char>(c));
                continue;
            }

            if (position >= source.size())
            {
                fail("Unterminated escape sequence");
                return false;
            }

            const char escaped = source[position++];
            if (escaped == '"' || escaped == '\\' || escaped == '/')
            {
                out.push_back(escaped);
            }
            else if (escaped == 'n')
            {
                out.push_back('\n');
            }
            else if (escaped == 'r')
            {
                out.push_back('\r');
            }
            else if (escaped == 't')
            {
                out.push_back('\t');
            }
            else if (escaped == 'b')
            {
                out.push_back('\b');
            }
            else if (escaped == 'f')
            {
                out.push_back('\f');
            }
            else
            {
                fail("Unsupported string escape (Unicode escapes are not supported)");
                return false;
            }
        }

        fail("Unterminated string");
        return false;
    }

    bool consumeLiteral(const char* literal)
    {
        int i = 0;

        while (literal[i] != '\0' && position + i < source.size() && source[position + i] == literal[i])
        {
            i++;
        }

        if (literal[i] != '\0')
        {
            return false;
        }

        position += i;
        return true;
    }

    JsonValue* parseInteger()
    {
        const int start = position;
        bool negative = false;

        if (source[position] == '-')
        {
            negative = true;
            position++;
        }

        if (position >= source.size() || source[position] < '0' || source[position] > '9')
        {
            return fail("Expected an integer digit");
        }

        if (source[position] == '0' && position + 1 < source.size() && source[position + 1] >= '0' && source[position + 1] <= '9')
        {
            return fail("Leading zeroes are not allowed");
        }

        long long number = 0;

        while (position < source.size() && source[position] >= '0' && source[position] <= '9')
        {
            const long long limit = static_cast<long long>(INT_MAX) + (negative ? 1 : 0);
            const long long digit = source[position] - '0';

            if (number > (limit - digit) / 10)
            {
                return fail("Integer is outside the supported range");
            }

            number = number * 10 + digit;
            position++;
        }

        if (position < source.size() && (source[position] == '.' || source[position] == 'e' || source[position] == 'E'))
        {
            return fail("Only integers are supported");
        }

        const int signedValue = negative
                                    ? (number == static_cast<long long>(INT_MAX) + 1 ? INT_MIN : -static_cast<int>(number))
                                    : static_cast<int>(number);

        JsonValue* value = new JsonValue(JsonType::Integer);
        value->m_integer = signedValue;

        if (position == start)
        {
            delete value;
            return fail("Expected an integer");
        }

        return value;
    }
};

bool JsonParser::parse(const std::string& text, JsonDocument& document, JsonError* error)
{
    document.clear();

    JsonParserState state(text, error);
    JsonValue* root = state.parseValue(0);

    if (root == nullptr)
    {
        return false;
    }
    state.skipWhitespace();

    if (state.position != text.size())
    {
        delete root;
        state.fail("Unexpected trailing data");
        return false;
    }

    document.m_root = root;
    return true;
}

std::string JsonWriter::quote(const std::string& value)
{
    std::string result;
    result.push_back('"');

    for (int i = 0; i < static_cast<int>(value.size()); i++)
    {
        const unsigned char c = static_cast<unsigned char>(value[i]);

        if (c == '"')
        {
            result += "\\\"";
        }
        else if (c == '\\')
        {
            result += "\\\\";
        }
        else if (c == '\n')
        {
            result += "\\n";
        }
        else if (c == '\r')
        {
            result += "\\r";
        }
        else if (c == '\t')
        {
            result += "\\t";
        }
        else if (c == '\b')
        {
            result += "\\b";
        }
        else if (c == '\f')
        {
            result += "\\f";
        }
        else if (c < 0x20)
        {
            const char hex[] = "0123456789abcdef";
            result += "\\u00";
            result.push_back(hex[(c >> 4) & 0xf]);
            result.push_back(hex[c & 0xf]);
        }
        else
        {
            result.push_back(static_cast<char>(c));
        }
    }

    result.push_back('"');
    return result;
}
