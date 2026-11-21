#ifndef VH_AVLTREE_H
#define VH_AVLTREE_H

#include "../contracts/Containers.h"
#include "../contracts/Event.h"
#include "../contracts/NodeDefs.h"
#include "EventLog.h"
#include "NodePool.h"


class AVLTree
{
  public:
    AVLTree();
    ~AVLTree();

    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;

    AVLNode* root() const;
    int nodeCount() const;
    bool isAlive(int nodeId) const;
    const NodePtrArray& pool() const;

    bool insert(int key, EventLog* log = nullptr);
    bool remove(int key, EventLog* log = nullptr);
    AVLNode* find(int key, EventLog* log = nullptr) const;
    bool contains(int key) const;

    static int height(const AVLNode* node);
    static int balanceFactor(const AVLNode* node);
    int height() const;

    int kth(int k) const;
    bool kth(int k, int& outKey) const;
    int rank(int key) const; 
    int countInRange(int lo, int hi) const;
    bool isBalanced() const;

    bool rotateLeft(AVLNode* pivot, EventLog* log = nullptr);
    bool rotateRight(AVLNode* pivot, EventLog* log = nullptr);

    void clear();

  private:
    friend class Serializer;
    AVLNode* findWithoutEvents(int key) const;
    AVLNode* rotateLeftRaw(AVLNode* pivot, EventLog* log);
    AVLNode* rotateRightRaw(AVLNode* pivot, EventLog* log);
    void rebalanceFrom(AVLNode* start, EventLog* log);
    void updateMetadata(AVLNode* node, EventLog* log);
    int countLessThan(int key) const;
    bool checkBalanced(const AVLNode* node) const;
    bool owns(const AVLNode* node) const;

    NodePool m_pool;
    AVLNode* m_root;
};

#endif
