
#pragma once
#include <iostream>
using namespace std;

class BST
{
public:
    struct Node
    {
        int data;
        int subtreeSize;
        Node *left;
        Node *right;

        Node(int value)
        {
            data = value;
            subtreeSize = 1;
            left = nullptr;
            right = nullptr;
        }
    };

    Node *root;

    BST()
    {
        root = nullptr;
    }

    int getSize(Node *node)
    {
        if (node == nullptr)
            return 0;

        return node->subtreeSize;
    }

    void updateSize(Node *node)
    {
        if (node != nullptr)
            node->subtreeSize = 1 + getSize(node->left) + getSize(node->right);
    }

    Node *insertNode(Node *node, int value)
    {
        if (node == nullptr)
            return new Node(value);

        if (value < node->data)
            node->left = insertNode(node->left, value);

        else if (value > node->data)
            node->right = insertNode(node->right, value);

        updateSize(node);

        return node;
    }

    void insert(int value)
    {
        root = insertNode(root, value);
    }

    bool search(Node *node, int value)
    {
        if (node == nullptr)
            return false;

        if (node->data == value)
            return true;

        if (value < node->data)
            return search(node->left, value);

        return search(node->right, value);
    }

    Node *findMin(Node *node)
    {
        while (node != nullptr && node->left != nullptr)
            node = node->left;

        return node;
    }

    Node *deleteNode(Node *node, int value)
    {
        if (node == nullptr)
            return nullptr;

        if (value < node->data)
            node->left = deleteNode(node->left, value);

        else if (value > node->data)
            node->right = deleteNode(node->right, value);

        else
        {
            if (node->left == nullptr)
            {
                Node *temp = node->right;
                delete node;
                return temp;
            }

            if (node->right == nullptr)
            {
                Node *temp = node->left;
                delete node;
                return temp;
            }

            Node *temp = findMin(node->right);
            node->data = temp->data;
            node->right = deleteNode(node->right, temp->data);
        }

        updateSize(node);

        return node;
    }

    void remove(int value)
    {
        root = deleteNode(root, value);
    }

    int kthSmallest(Node *node, int k)
    {
        if (node == nullptr || k < 1 || k > getSize(node))
            return -1;

        int leftSize = getSize(node->left);

        if (k == leftSize + 1)
            return node->data;

        if (k <= leftSize)
            return kthSmallest(node->left, k);

        return kthSmallest(node->right, k - leftSize - 1);
    }

    void inorder(Node *node)
    {
        if (node == nullptr)
            return;

        inorder(node->left);
        cout << node->data << " ";
        inorder(node->right);
    }

    void preorder(Node *node)
    {
        if (node == nullptr)
            return;

        cout << node->data << " ";
        preorder(node->left);
        preorder(node->right);
    }

    void postorder(Node *node)
    {
        if (node == nullptr)
            return;

        postorder(node->left);
        postorder(node->right);
        cout << node->data << " ";
    }

    int height(Node *node)
    {
        if (node == nullptr)
            return -1;

        int lh = height(node->left);
        int rh = height(node->right);

        return 1 + max(lh, rh);
    }

    int countNodes(Node *node)
    {
        if (node == nullptr)
            return 0;

        return 1 + countNodes(node->left) + countNodes(node->right);
    }

    void destroy(Node *node)
    {
        if (node == nullptr)
            return;

        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    ~BST()
    {
        destroy(root);
    }
};
