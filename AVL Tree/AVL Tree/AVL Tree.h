
#pragma once
#include <iostream>
using namespace std;

class BinaryTree
{
public:
    struct Node
    {
        int data;
        Node *left;
        Node *right;

        Node(int value)
        {
            data = value;
            left = nullptr;
            right = nullptr;
        }
    };

    Node *root;

    BinaryTree()
    {
        root = nullptr;
    }

    Node *buildTree(int preorder[], int inorder[], int start, int end, int &index)
    {
        if (start > end)
            return nullptr;

        Node *node = new Node(preorder[index++]);

        if (start == end)
            return node;

        int pos = start;

        while (pos <= end && inorder[pos] != node->data)
            pos++;

        node->left = buildTree(preorder, inorder, start, pos - 1, index);
        node->right = buildTree(preorder, inorder, pos + 1, end, index);

        return node;
    }

    void reconstruct(int preorder[], int inorder[], int size)
    {
        int index = 0;
        root = buildTree(preorder, inorder, 0, size - 1, index);
    }

    void preorder(Node *node)
    {
        if (node == nullptr)
            return;

        cout << node->data << " ";
        preorder(node->left);
        preorder(node->right);
    }

    void inorder(Node *node)
    {
        if (node == nullptr)
            return;

        inorder(node->left);
        cout << node->data << " ";
        inorder(node->right);
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

        int leftHeight = height(node->left);
        int rightHeight = height(node->right);

        return 1 + max(leftHeight, rightHeight);
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

    ~BinaryTree()
    {
        destroy(root);
    }
};
