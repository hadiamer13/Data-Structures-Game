#ifndef VH_BINARYTREE_H
#define VH_BINARYTREE_H

#include "../contracts/Containers.h"
#include "../contracts/Event.h"
#include "../contracts/NodeDefs.h"
#include "EventLog.h"
#include "NodePool.h"

// General binary tree with no ordering requirement. Node IDs are pool indices.
// height() counts nodes; diameter() counts edges on the longest path.
class BinaryTree
{
public:
    BinaryTree();
    ~BinaryTree();

    BinaryTree(const BinaryTree&) = delete;
    BinaryTree& operator=(const BinaryTree&) = delete;

    TreeNode* root() const;
    int nodeCount() const;
    bool isAlive(int nodeId) const;
    const NodePtrArray& pool() const;

    TreeNode* createNode(int value);
    bool setRoot(TreeNode* node, EventLog* log = nullptr);
    bool attachLeft(TreeNode* parent, TreeNode* child, EventLog* log = nullptr);
    bool attachRight(TreeNode* parent, TreeNode* child, EventLog* log = nullptr);

    bool build(const IntArray& preorderValues, const IntArray& inorderValues, EventLog* log = nullptr);
    bool buildFromPreIn(const IntArray& preorderValues, const IntArray& inorderValues, EventLog* log = nullptr);
    bool buildFromPostIn(const IntArray& postorderValues, const IntArray& inorderValues, EventLog* log = nullptr);

    IntArray preorderRecursive(EventLog* log = nullptr) const;
    IntArray preorderIterative(EventLog* log = nullptr) const;
    IntArray inorderRecursive(EventLog* log = nullptr) const;
    IntArray inorderIterative(EventLog* log = nullptr) const;
    IntArray postorderRecursive(EventLog* log = nullptr) const;
    IntArray postorderIterative(EventLog* log = nullptr) const;
    IntArray levelOrderRecursive(EventLog* log = nullptr) const;
    IntArray levelOrderIterative(EventLog* log = nullptr) const;

    IntArray preorder(EventLog* log = nullptr) const;
    IntArray inorder(EventLog* log = nullptr) const;
    IntArray postorder(EventLog* log = nullptr) const;
    IntArray levelOrder(EventLog* log = nullptr) const;

    int height() const;
    static int height(const TreeNode* node);
    int diameter() const;
    int lca(int nodeIdA, int nodeIdB, EventLog* log = nullptr) const;
    IntArray pathTo(int nodeId, EventLog* log = nullptr) const;
    bool pathTo(int nodeId, IntArray& outIds, EventLog* log = nullptr) const;

    void mirror(EventLog* log = nullptr);
    bool isSameTree(const BinaryTree& other) const;
    bool isSymmetric() const;
    void clear();

private:
    bool owns(const TreeNode* node) const;
    bool isReachable(const TreeNode* target) const;
    bool attachChild(TreeNode* parent, TreeNode* child, int side, EventLog* log);

    void preorderRec(TreeNode* node, IntArray& out, EventLog* log) const;
    void inorderRec(TreeNode* node, IntArray& out, EventLog* log) const;
    void postorderRec(TreeNode* node, IntArray& out, EventLog* log) const;
    void levelAtDepth(TreeNode* node, int depth, int targetDepth, IntArray& out, EventLog* log) const;
    int diameterHeight(TreeNode* node, int& bestEdges) const;
    TreeNode* lcaRec(TreeNode* node, int nodeIdA, int nodeIdB, EventLog* log) const;
    bool pathRec(TreeNode* node, int targetId, IntArray& outIds, EventLog* log) const;
    void mirrorRec(TreeNode* node, EventLog* log);
    bool sameRec(const TreeNode* a, const TreeNode* b) const;
    bool symmetricRec(const TreeNode* left, const TreeNode* right) const;

    TreeNode* buildPreRec(const IntArray& preorderValues, const IntArray& inorderValues,
                          int& preorderIndex, int inLeft, int inRight,
                          TreeNode* parent, int side, EventLog* log, bool& ok);
    TreeNode* buildPostRec(const IntArray& postorderValues, const IntArray& inorderValues,
                           int& postorderIndex, int inLeft, int inRight,
                           TreeNode* parent, int side, EventLog* log, bool& ok);
    bool validTraversalInputs(const IntArray& order, const IntArray& inorderValues) const;

    NodePool m_pool;
    TreeNode* m_root;
};

#endif
