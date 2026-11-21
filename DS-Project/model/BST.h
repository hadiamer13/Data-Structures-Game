#ifndef VH_BST_H
#define VH_BST_H

#include "NodePool.h"
#include "EventLog.h"


class BST
{
  public:
    BST();
    ~BST();

    BST(const BST&) = delete;
    BST& operator=(const BST&) = delete;

    BSTNode* root() const;
    int nodeCount() const;
    bool isAlive(int nodeId) const;
    const NodePtrArray& pool() const;

    bool insert(int key, EventLog* log = nullptr);
    bool remove(int key, EventLog* log = nullptr);
    BSTNode* search(int key, EventLog* log = nullptr) const;
    BSTNode* findMin() const;
    BSTNode* findMax() const;
    BSTNode* successor(int key) const;
    BSTNode* predecessor(int key) const;
    void rangeQuery(int low, int high, IntArray& out, EventLog* log = nullptr) const;
    IntArray rangeQuery(int low, int high, EventLog* log = nullptr) const;
    int kthSmallest(int k) const;
    void clear();

  private:
    void emitSkipped(BSTNode* subtreeRoot, EventLog* log) const;
    friend class Serializer;
    bool owns(const BSTNode* node) const;
    BSTNode* findWithoutEvents(int key) const;
    void rangeRec(BSTNode* node, int low, int high, IntArray& out, EventLog* log) const;

    NodePool m_pool;
    BSTNode* m_root;
};

#endif
