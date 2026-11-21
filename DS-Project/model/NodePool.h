// model/NodePool.h

#pragma once

#include "../contracts/Types.h"
#include "../contracts/Containers.h"
#include "../contracts/NodeDefs.h"

class NodePool
{
  public:
    NodePool()
    {
    }

    ~NodePool()
    {
        clear();
    }

    TreeNode* createTree(int value)
    {
        TreeNode* n = new TreeNode(m_nodes.size(), value);
        add(n);
        return n;
    }

    BSTNode* createBST(int key)
    {
        BSTNode* n = new BSTNode(m_nodes.size(), key);
        add(n);
        return n;
    }

    AVLNode* createAVL(int key)
    {
        AVLNode* n = new AVLNode(m_nodes.size(), key);
        add(n);
        return n;
    }

    TreeNode* get(int id) const
    {
        if (id < 0 || id >= m_nodes.size())
        {
            return nullptr;
        }
        return m_nodes[id];
    }

    int size() const
    {
        return m_nodes.size();
    }

    int aliveCount() const
    {
        int c = 0;
        for (int i = 0; i < m_alive.size(); i++)
        {
            if (m_alive[i])
            {
                c = c + 1;
            }
        }
        return c;
    }

    bool isAlive(int id) const
    {
        if (id < 0 || id >= m_alive.size())
        {
            return false;
        }
        return m_alive[id];
    }

    void markDead(int id)
    {
        if (id >= 0 && id < m_alive.size())
        {
            m_alive[id] = false;
        }
    }

    void markAlive(int id)
    {
        if (id >= 0 && id < m_alive.size())
        {
            m_alive[id] = true;
        }
    }

    const NodePtrArray& pool() const
    {
        return m_nodes;
    }

    void clear()
    {
        for (int i = 0; i < m_nodes.size(); i++)
        {
            delete m_nodes[i];
        }
        m_nodes.clear();
        m_alive.clear();
    }

  private:
    NodePool(const NodePool&) = delete;
    NodePool& operator=(const NodePool&) = delete;

    void add(TreeNode* n)
    {
        m_nodes.push(n);
        m_alive.push(true);
    }

    NodePtrArray m_nodes;
    BoolArray m_alive;
};
