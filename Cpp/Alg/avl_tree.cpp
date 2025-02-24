#include <iostream>

#include "tree_node.h"

using namespace std;

int height(AVLTreeNode* node);
void update_height(AVLTreeNode* node);
int balance_factor(AVLTreeNode* node);
AVLTreeNode* right_rotate(AVLTreeNode* node);
AVLTreeNode* left_rotate(AVLTreeNode* node);
AVLTreeNode* rotate(AVLTreeNode* node);
AVLTreeNode* insert_helper(AVLTreeNode* node, int val);
AVLTreeNode* remove_helper(AVLTreeNode* node, int val);
void print_node(AVLTreeNode* node);

int main()
{
    AVLTreeNode* root = new AVLTreeNode(0);
    root = insert_helper(root, 4);
    root = insert_helper(root, 2);
    root = insert_helper(root, 6);
    root = insert_helper(root, 1);
    root = insert_helper(root, 3);
    root = insert_helper(root, 4);
    root = insert_helper(root, 7);
    print_node(root);
    cout << "Delete: " << endl;
    root = remove_helper(root, 4);
    print_node(root);
}

int height(AVLTreeNode* node)
{
    return node == nullptr ? -1 : node->height;
}

void update_height(AVLTreeNode* node)
{
    node->height = max(height(node->left), height(node->right)) + 1;
}

int balance_factor(AVLTreeNode* node)
{
    if (node == nullptr) return 0;

    return height(node->left) - height(node->right);
}

AVLTreeNode* right_rotate(AVLTreeNode* node)
{
    AVLTreeNode* child = node->left;
    AVLTreeNode* grand_child = child->right;
    child->right = node;
    node->left = grand_child;

    update_height(node);
    update_height(child);
    return child;
}

AVLTreeNode* left_rotate(AVLTreeNode* node)
{
    AVLTreeNode* child = node->right;
    AVLTreeNode* grand_child = child->left;
    child->left = node;
    node->right = grand_child;

    update_height(node);
    update_height(child);
    return child;
}

AVLTreeNode* rotate(AVLTreeNode* node)
{
    int _balance_factor = balance_factor(node);

    if (_balance_factor > 1)
    {
        if (balance_factor(node->left) >= 0)
        {
            return right_rotate(node);
        }
        else
        {
            node->left = left_rotate(node->left);
            return right_rotate(node);
        }
    }

    if (_balance_factor < -1)
    {
        if (balance_factor(node->right) <= 0)
        {
            return left_rotate(node);
        }
        else
        {
            node->right = right_rotate(node->right);
            return left_rotate(node);
        }
    }

    return node;
}


AVLTreeNode* insert_helper(AVLTreeNode* node, int val)
{
    if (node == nullptr) return new AVLTreeNode(val);

    if (val < node->val)
    {
        node->left = insert_helper(node->left, val);
    }
    else if (val > node->val)
    {
        node->right = insert_helper(node->right, val);
    }
    else
    {
        return node;
    }

    update_height(node);
    node = rotate(node);
    return node;
}

AVLTreeNode* remove_helper(AVLTreeNode* node, int val)
{
    if (node == nullptr) return nullptr;

    if (val < node->val)
        node->left = remove_helper(node->left, val);
    else if (val > node->val)
        node->right = remove_helper(node->right, val);
    else 
    {
        if (node->left == nullptr || node->right == nullptr) 
        {
            AVLTreeNode *child = node->left != nullptr ? node->left : node->right;

            if (child == nullptr) 
            {
                delete node;
                return nullptr;
            }
            else 
            {
                delete node;
                node = child;
            }
        } 
        else 
        {

            AVLTreeNode *temp = node->right;
            while (temp->left != nullptr) 
            {
                temp = temp->left;
            }
            int tempVal = temp->val;
            node->right = remove_helper(node->right, temp->val);
            node->val = tempVal;
        }
    }

    update_height(node);
    node = rotate(node);
    return node;
}

void print_node(AVLTreeNode* node)
{
    if (node == nullptr) return;

    if (node->left != nullptr) print_node(node->left);

    cout << "[val, height, factor]: " << node->val << ", " << node->height << ", " << balance_factor(node) << endl; 

    if (node->right != nullptr) print_node(node->right);
}
