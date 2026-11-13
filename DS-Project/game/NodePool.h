// model/NodePool.h
// Owns every node of one tree. Raw pointers, unique dense ids (id == pool index).
// Ids are never reused and never change on rotation or value swap (frozen decision 3).
// Removed nodes stay in the pool (marked dead) so undo can relink them and ids never shift.
// The destructor deletes every node.
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

    // One creator per node type (no templates). Each uses the next id.
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

    // Node by id, or nullptr for kNull / out of range.
    TreeNode* get(int id) const
    {
        if (id < 0 || id >= m_nodes.size())
        {
            return nullptr;
        }
        return m_nodes[id];
    }

    // Total slots ever created (dead ones included). Equals the next id to be handed out.
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

    // Marks a node removed from the tree (it is NOT deleted; undo may revive it).
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

    // Read access for the validator / snapshot builder (frozen decision 3: pool() lookup).
    const NodePtrArray& pool() const
    {
        return m_nodes;
    }

    // Deletes every node and restarts ids at 0.
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
    // Owning raw pointers: copying would double-delete.
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
