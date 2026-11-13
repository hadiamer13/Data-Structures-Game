// contracts/Containers.h
// Own NON-template containers. One concrete class per element type, all built from the
// same pattern: raw new[]/delete[], doubling growth, deep copy (rule of three).
// operator[] / back() / top() / front() are UNCHECKED: test size()/empty() first.
// ViewNodeArray lives in ViewSnapshot.h (it needs ViewNode). /game adds JsonPtrArray and
// LevelDefArray the same way.
#ifndef VH_CONTAINERS_H
#define VH_CONTAINERS_H

#include <string>
#include "Types.h"
#include "Event.h"
#include "NodeDefs.h"

// Typedef so "const TreeNodePtr&" means a const reference to a pointer (not pointer-to-const).
typedef TreeNode* TreeNodePtr;

class IntArray
{
public:
    IntArray() : m_data(nullptr), m_size(0), m_cap(0)
    {
    }

    IntArray(const IntArray& other) : m_data(nullptr), m_size(0), m_cap(0)
    {
        copyFrom(other);
    }

    IntArray& operator=(const IntArray& other)
    {
        if (this != &other)
        {
            copyFrom(other);
        }
        return *this;
    }

    ~IntArray()
    {
        delete[] m_data;
    }

    int size() const
    {
        return m_size;
    }

    int capacity() const
    {
        return m_cap;
    }

    bool empty() const
    {
        return m_size == 0;
    }

    int& operator[](int i)
    {
        return m_data[i];
    }

    const int& operator[](int i) const
    {
        return m_data[i];
    }

    int& back()
    {
        return m_data[m_size - 1];
    }

    const int& back() const
    {
        return m_data[m_size - 1];
    }

    int* data()
    {
        return m_data;
    }

    const int* data() const
    {
        return m_data;
    }

    void push(const int& v)
    {
        if (m_size == m_cap)
        {
            reserve(m_cap == 0 ? 4 : m_cap * 2);
        }
        m_data[m_size] = v;
        m_size = m_size + 1;
    }

    void pop()
    {
        if (m_size > 0)
        {
            m_size = m_size - 1;
        }
    }

    void clear()
    {
        m_size = 0;
    }

    void reserve(int n)
    {
        if (n <= m_cap)
        {
            return;
        }
        int* nd = new int[n];
        for (int i = 0; i < m_size; i++)
        {
            nd[i] = m_data[i];
        }
        delete[] m_data;
        m_data = nd;
        m_cap = n;
    }

    void resize(int n)
    {
        reserve(n);
        for (int i = m_size; i < n; i++)
        {
            m_data[i] = 0;
        }
        m_size = n;
    }

    int indexOf(const int& v) const
    {
        for (int i = 0; i < m_size; i++)
        {
            if (m_data[i] == v)
            {
                return i;
            }
        }
        return -1;
    }
private:
    void copyFrom(const IntArray& other)
    {
        int* nd = nullptr;
        if (other.m_size > 0)
        {
            nd = new int[other.m_size];
            for (int i = 0; i < other.m_size; i++)
            {
                nd[i] = other.m_data[i];
            }
        }
        delete[] m_data;
        m_data = nd;
        m_size = other.m_size;
        m_cap = other.m_size;
    }

    int* m_data;
    int m_size;
    int m_cap;
};

class BoolArray
{
public:
    BoolArray() : m_data(nullptr), m_size(0), m_cap(0)
    {
    }

    BoolArray(const BoolArray& other) : m_data(nullptr), m_size(0), m_cap(0)
    {
        copyFrom(other);
    }

    BoolArray& operator=(const BoolArray& other)
    {
        if (this != &other)
        {
            copyFrom(other);
        }
        return *this;
    }

    ~BoolArray()
    {
        delete[] m_data;
    }

    int size() const
    {
        return m_size;
    }

    int capacity() const
    {
        return m_cap;
    }

    bool empty() const
    {
        return m_size == 0;
    }

    bool& operator[](int i)
    {
        return m_data[i];
    }

    const bool& operator[](int i) const
    {
        return m_data[i];
    }

    bool& back()
    {
        return m_data[m_size - 1];
    }

    const bool& back() const
    {
        return m_data[m_size - 1];
    }

    bool* data()
    {
        return m_data;
    }

    const bool* data() const
    {
        return m_data;
    }

    void push(const bool& v)
    {
        if (m_size == m_cap)
        {
            reserve(m_cap == 0 ? 4 : m_cap * 2);
        }
        m_data[m_size] = v;
        m_size = m_size + 1;
    }

    void pop()
    {
        if (m_size > 0)
        {
            m_size = m_size - 1;
        }
    }

    void clear()
    {
        m_size = 0;
    }

    void reserve(int n)
    {
        if (n <= m_cap)
        {
            return;
        }
        bool* nd = new bool[n];
        for (int i = 0; i < m_size; i++)
        {
            nd[i] = m_data[i];
        }
        delete[] m_data;
        m_data = nd;
        m_cap = n;
    }

    void resize(int n)
    {
        reserve(n);
        for (int i = m_size; i < n; i++)
        {
            m_data[i] = false;
        }
        m_size = n;
    }

private:
    void copyFrom(const BoolArray& other)
    {
        bool* nd = nullptr;
        if (other.m_size > 0)
        {
            nd = new bool[other.m_size];
            for (int i = 0; i < other.m_size; i++)
            {
                nd[i] = other.m_data[i];
            }
        }
        delete[] m_data;
        m_data = nd;
        m_size = other.m_size;
        m_cap = other.m_size;
    }

    bool* m_data;
    int m_size;
    int m_cap;
};

class NodePtrArray
{
public:
    NodePtrArray() : m_data(nullptr), m_size(0), m_cap(0)
    {
    }

    NodePtrArray(const NodePtrArray& other) : m_data(nullptr), m_size(0), m_cap(0)
    {
        copyFrom(other);
    }

    NodePtrArray& operator=(const NodePtrArray& other)
    {
        if (this != &other)
        {
            copyFrom(other);
        }
        return *this;
    }

    ~NodePtrArray()
    {
        delete[] m_data;
    }

    int size() const
    {
        return m_size;
    }

    int capacity() const
    {
        return m_cap;
    }

    bool empty() const
    {
        return m_size == 0;
    }

    TreeNodePtr& operator[](int i)
    {
        return m_data[i];
    }

    const TreeNodePtr& operator[](int i) const
    {
        return m_data[i];
    }

    TreeNodePtr& back()
    {
        return m_data[m_size - 1];
    }

    const TreeNodePtr& back() const
    {
        return m_data[m_size - 1];
    }

    TreeNodePtr* data()
    {
        return m_data;
    }

    const TreeNodePtr* data() const
    {
        return m_data;
    }

    void push(const TreeNodePtr& v)
    {
        if (m_size == m_cap)
        {
            reserve(m_cap == 0 ? 4 : m_cap * 2);
        }
        m_data[m_size] = v;
        m_size = m_size + 1;
    }

    void pop()
    {
        if (m_size > 0)
        {
            m_size = m_size - 1;
        }
    }

    void clear()
    {
        m_size = 0;
    }

    void reserve(int n)
    {
        if (n <= m_cap)
        {
            return;
        }
        TreeNodePtr* nd = new TreeNodePtr[n];
        for (int i = 0; i < m_size; i++)
        {
            nd[i] = m_data[i];
        }
        delete[] m_data;
        m_data = nd;
        m_cap = n;
    }

    void resize(int n)
    {
        reserve(n);
        for (int i = m_size; i < n; i++)
        {
            m_data[i] = nullptr;
        }
        m_size = n;
    }

    int indexOf(const TreeNodePtr& v) const
    {
        for (int i = 0; i < m_size; i++)
        {
            if (m_data[i] == v)
            {
                return i;
            }
        }
        return -1;
    }
private:
    void copyFrom(const NodePtrArray& other)
    {
        TreeNodePtr* nd = nullptr;
        if (other.m_size > 0)
        {
            nd = new TreeNodePtr[other.m_size];
            for (int i = 0; i < other.m_size; i++)
            {
                nd[i] = other.m_data[i];
            }
        }
        delete[] m_data;
        m_data = nd;
        m_size = other.m_size;
        m_cap = other.m_size;
    }

    TreeNodePtr* m_data;
    int m_size;
    int m_cap;
};

class EventArray
{
public:
    EventArray() : m_data(nullptr), m_size(0), m_cap(0)
    {
    }

    EventArray(const EventArray& other) : m_data(nullptr), m_size(0), m_cap(0)
    {
        copyFrom(other);
    }

    EventArray& operator=(const EventArray& other)
    {
        if (this != &other)
        {
            copyFrom(other);
        }
        return *this;
    }

    ~EventArray()
    {
        delete[] m_data;
    }

    int size() const
    {
        return m_size;
    }

    int capacity() const
    {
        return m_cap;
    }

    bool empty() const
    {
        return m_size == 0;
    }

    Event& operator[](int i)
    {
        return m_data[i];
    }

    const Event& operator[](int i) const
    {
        return m_data[i];
    }

    Event& back()
    {
        return m_data[m_size - 1];
    }

    const Event& back() const
    {
        return m_data[m_size - 1];
    }

    Event* data()
    {
        return m_data;
    }

    const Event* data() const
    {
        return m_data;
    }

    void push(const Event& v)
    {
        if (m_size == m_cap)
        {
            reserve(m_cap == 0 ? 4 : m_cap * 2);
        }
        m_data[m_size] = v;
        m_size = m_size + 1;
    }

    void pop()
    {
        if (m_size > 0)
        {
            m_size = m_size - 1;
        }
    }

    void clear()
    {
        m_size = 0;
    }

    void reserve(int n)
    {
        if (n <= m_cap)
        {
            return;
        }
        Event* nd = new Event[n];
        for (int i = 0; i < m_size; i++)
        {
            nd[i] = m_data[i];
        }
        delete[] m_data;
        m_data = nd;
        m_cap = n;
    }

    void resize(int n)
    {
        reserve(n);
        for (int i = m_size; i < n; i++)
        {
            m_data[i] = Event();
        }
        m_size = n;
    }

private:
    void copyFrom(const EventArray& other)
    {
        Event* nd = nullptr;
        if (other.m_size > 0)
        {
            nd = new Event[other.m_size];
            for (int i = 0; i < other.m_size; i++)
            {
                nd[i] = other.m_data[i];
            }
        }
        delete[] m_data;
        m_data = nd;
        m_size = other.m_size;
        m_cap = other.m_size;
    }

    Event* m_data;
    int m_size;
    int m_cap;
};

class StringArray
{
public:
    StringArray() : m_data(nullptr), m_size(0), m_cap(0)
    {
    }

    StringArray(const StringArray& other) : m_data(nullptr), m_size(0), m_cap(0)
    {
        copyFrom(other);
    }

    StringArray& operator=(const StringArray& other)
    {
        if (this != &other)
        {
            copyFrom(other);
        }
        return *this;
    }

    ~StringArray()
    {
        delete[] m_data;
    }

    int size() const
    {
        return m_size;
    }

    int capacity() const
    {
        return m_cap;
    }

    bool empty() const
    {
        return m_size == 0;
    }

    std::string& operator[](int i)
    {
        return m_data[i];
    }

    const std::string& operator[](int i) const
    {
        return m_data[i];
    }

    std::string& back()
    {
        return m_data[m_size - 1];
    }

    const std::string& back() const
    {
        return m_data[m_size - 1];
    }

    std::string* data()
    {
        return m_data;
    }

    const std::string* data() const
    {
        return m_data;
    }

    void push(const std::string& v)
    {
        if (m_size == m_cap)
        {
            reserve(m_cap == 0 ? 4 : m_cap * 2);
        }
        m_data[m_size] = v;
        m_size = m_size + 1;
    }

    void pop()
    {
        if (m_size > 0)
        {
            m_size = m_size - 1;
        }
    }

    void clear()
    {
        m_size = 0;
    }

    void reserve(int n)
    {
        if (n <= m_cap)
        {
            return;
        }
        std::string* nd = new std::string[n];
        for (int i = 0; i < m_size; i++)
        {
            nd[i] = m_data[i];
        }
        delete[] m_data;
        m_data = nd;
        m_cap = n;
    }

    void resize(int n)
    {
        reserve(n);
        for (int i = m_size; i < n; i++)
        {
            m_data[i] = std::string();
        }
        m_size = n;
    }

private:
    void copyFrom(const StringArray& other)
    {
        std::string* nd = nullptr;
        if (other.m_size > 0)
        {
            nd = new std::string[other.m_size];
            for (int i = 0; i < other.m_size; i++)
            {
                nd[i] = other.m_data[i];
            }
        }
        delete[] m_data;
        m_data = nd;
        m_size = other.m_size;
        m_cap = other.m_size;
    }

    std::string* m_data;
    int m_size;
    int m_cap;
};

class IntStack
{
public:
    bool empty() const
    {
        return m_items.empty();
    }

    int size() const
    {
        return m_items.size();
    }

    void push(const int& v)
    {
        m_items.push(v);
    }

    int& top()
    {
        return m_items.back();
    }

    const int& top() const
    {
        return m_items.back();
    }

    void pop()
    {
        m_items.pop();
    }

    void clear()
    {
        m_items.clear();
    }

private:
    IntArray m_items;
};

class NodeStack
{
public:
    bool empty() const
    {
        return m_items.empty();
    }

    int size() const
    {
        return m_items.size();
    }

    void push(const TreeNodePtr& v)
    {
        m_items.push(v);
    }

    TreeNodePtr& top()
    {
        return m_items.back();
    }

    const TreeNodePtr& top() const
    {
        return m_items.back();
    }

    void pop()
    {
        m_items.pop();
    }

    void clear()
    {
        m_items.clear();
    }

private:
    NodePtrArray m_items;
};

class IntQueue
{
public:
    IntQueue() : m_data(nullptr), m_head(0), m_size(0), m_cap(0)
    {
    }

    IntQueue(const IntQueue& other) : m_data(nullptr), m_head(0), m_size(0), m_cap(0)
    {
        copyFrom(other);
    }

    IntQueue& operator=(const IntQueue& other)
    {
        if (this != &other)
        {
            copyFrom(other);
        }
        return *this;
    }

    ~IntQueue()
    {
        delete[] m_data;
    }

    bool empty() const
    {
        return m_size == 0;
    }

    int size() const
    {
        return m_size;
    }

    void push(const int& v)
    {
        if (m_size == m_cap)
        {
            grow(m_cap == 0 ? 4 : m_cap * 2);
        }
        m_data[(m_head + m_size) % m_cap] = v;
        m_size = m_size + 1;
    }

    int& front()
    {
        return m_data[m_head];
    }

    const int& front() const
    {
        return m_data[m_head];
    }

    void pop()
    {
        if (m_size > 0)
        {
            m_head = (m_head + 1) % m_cap;
            m_size = m_size - 1;
        }
    }

    void clear()
    {
        m_head = 0;
        m_size = 0;
    }

private:
    void grow(int newCap)
    {
        int* nd = new int[newCap];
        for (int i = 0; i < m_size; i++)
        {
            nd[i] = m_data[(m_head + i) % m_cap];
        }
        delete[] m_data;
        m_data = nd;
        m_head = 0;
        m_cap = newCap;
    }

    void copyFrom(const IntQueue& other)
    {
        int* nd = nullptr;
        if (other.m_size > 0)
        {
            nd = new int[other.m_size];
            for (int i = 0; i < other.m_size; i++)
            {
                nd[i] = other.m_data[(other.m_head + i) % other.m_cap];
            }
        }
        delete[] m_data;
        m_data = nd;
        m_head = 0;
        m_size = other.m_size;
        m_cap = other.m_size;
    }

    int* m_data;
    int m_head;
    int m_size;
    int m_cap;
};

class NodeQueue
{
public:
    NodeQueue() : m_data(nullptr), m_head(0), m_size(0), m_cap(0)
    {
    }

    NodeQueue(const NodeQueue& other) : m_data(nullptr), m_head(0), m_size(0), m_cap(0)
    {
        copyFrom(other);
    }

    NodeQueue& operator=(const NodeQueue& other)
    {
        if (this != &other)
        {
            copyFrom(other);
        }
        return *this;
    }

    ~NodeQueue()
    {
        delete[] m_data;
    }

    bool empty() const
    {
        return m_size == 0;
    }

    int size() const
    {
        return m_size;
    }

    void push(const TreeNodePtr& v)
    {
        if (m_size == m_cap)
        {
            grow(m_cap == 0 ? 4 : m_cap * 2);
        }
        m_data[(m_head + m_size) % m_cap] = v;
        m_size = m_size + 1;
    }

    TreeNodePtr& front()
    {
        return m_data[m_head];
    }

    const TreeNodePtr& front() const
    {
        return m_data[m_head];
    }

    void pop()
    {
        if (m_size > 0)
        {
            m_head = (m_head + 1) % m_cap;
            m_size = m_size - 1;
        }
    }

    void clear()
    {
        m_head = 0;
        m_size = 0;
    }

private:
    void grow(int newCap)
    {
        TreeNodePtr* nd = new TreeNodePtr[newCap];
        for (int i = 0; i < m_size; i++)
        {
            nd[i] = m_data[(m_head + i) % m_cap];
        }
        delete[] m_data;
        m_data = nd;
        m_head = 0;
        m_cap = newCap;
    }

    void copyFrom(const NodeQueue& other)
    {
        TreeNodePtr* nd = nullptr;
        if (other.m_size > 0)
        {
            nd = new TreeNodePtr[other.m_size];
            for (int i = 0; i < other.m_size; i++)
            {
                nd[i] = other.m_data[(other.m_head + i) % other.m_cap];
            }
        }
        delete[] m_data;
        m_data = nd;
        m_head = 0;
        m_size = other.m_size;
        m_cap = other.m_size;
    }

    TreeNodePtr* m_data;
    int m_head;
    int m_size;
    int m_cap;
};

#endif
