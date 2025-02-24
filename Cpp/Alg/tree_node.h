#pragma once

struct TreeNode
{
    int m_val;
    TreeNode* m_left;
    TreeNode* m_right;
    TreeNode(int val) : m_val(val), m_left(nullptr), m_right(nullptr) {};
};

struct AVLTreeNode
{
    int val;
    int height = 0;
    AVLTreeNode* left = nullptr;
    AVLTreeNode* right = nullptr;
    AVLTreeNode() = default;
    explicit AVLTreeNode(int x) : val(x) {};
};

enum TraversalOrder
{
    BreadthFirst = 1 << 0,
    DepthFirstPreOrder = 1 << 1,
    DepthFirstInOrder = 1 << 2,
    DepthFirstPostOrder = 1 << 3,
};

void print_node(TreeNode* node, TraversalOrder order);
void print_node_iter(TreeNode* node, TraversalOrder order); // Just to show how to use iteration rather than recursion to depth first 

