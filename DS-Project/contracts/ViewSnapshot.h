// contracts/ViewSnapshot.h

#ifndef VH_VIEWSNAPSHOT_H
#define VH_VIEWSNAPSHOT_H

#include "Types.h"
#include "Containers.h"

struct ViewNode
{
    int id;
    int key;
    int left;
    int right;
    int parent;
    int height;
    int size;
    int balance;
    bool alive;

    ViewNode() : id(kNull), key(0), left(kNull), right(kNull), parent(kNull), height(0), size(0), balance(0), alive(false)
    {
    }
};

class ViewNodeArray
{
  public:
    ViewNodeArray() : m_data(nullptr), m_size(0), m_cap(0)
    {
    }

    ViewNodeArray(const ViewNodeArray& other) : m_data(nullptr), m_size(0), m_cap(0)
    {
        copyFrom(other);
    }

    ViewNodeArray& operator=(const ViewNodeArray& other)
    {
        if (this != &other)
        {
            copyFrom(other);
        }

        return *this;
    }

    ~ViewNodeArray()
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

    ViewNode& operator[](int i)
    {
        return m_data[i];
    }

    const ViewNode& operator[](int i) const
    {
        return m_data[i];
    }

    ViewNode& back()
    {
        return m_data[m_size - 1];
    }

    const ViewNode& back() const
    {
        return m_data[m_size - 1];
    }

    ViewNode* data()
    {
        return m_data;
    }

    const ViewNode* data() const
    {
        return m_data;
    }

    void push(const ViewNode& v)
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

        ViewNode* nd = new ViewNode[n];

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
            m_data[i] = ViewNode();
        }

        m_size = n;
    }

  private:
    void copyFrom(const ViewNodeArray& other)
    {
        ViewNode* nd = nullptr;

        if (other.m_size > 0)
        {
            nd = new ViewNode[other.m_size];

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

    ViewNode* m_data;
    int m_size;
    int m_cap;
};

struct ViewHeap
{
    IntArray array;
    int activeSize;

    ViewHeap() : activeSize(0)
    {
    }
};

struct ViewSnapshot
{
    int kind;
    int rootId;
    ViewNodeArray nodes;
    ViewHeap heap;

    ViewSnapshot() : kind(TreeKind::BST), rootId(kNull)
    {
    }
};

#endif
