/*
Build tree problem: 
    Given a binary's preorder and inorder, build the tree.
*/

#include <iostream>
#include <vector>
#include <unordered_map>

#include "tree_node.h"

using namespace std;

TreeNode* build_tree(vector<int> &pre_order, vector<int> &in_order);
// [l, r] is the current tree in the inorder, i is the current root in preorder, m is the current root in inorder
TreeNode* dfs(vector<int> &pre_order, unordered_map<int, int> &inorder_map, int i, int l, int r);

int main()
{
    vector<int> pre_order = {3, 9, 2, 1, 7};
    vector<int> in_order = {9, 3, 1, 2, 7};

    TreeNode* root = build_tree(pre_order, in_order);
    print_node(root, TraversalOrder::DepthFirstPostOrder);
}

TreeNode* build_tree(vector<int> &pre_order, vector<int> &in_order)
{
    unordered_map<int, int> mp;
    for (int i = 0; i < in_order.size(); ++i) mp[in_order[i]] = i;

    TreeNode* root = dfs(pre_order, mp, 0, 0, in_order.size() - 1);
    return root;
}

TreeNode* dfs(vector<int> &pre_order, unordered_map<int, int> &inorder_map, int i, int l, int r)
{
    if (r - l < 0) return nullptr;

    TreeNode* root = new TreeNode(pre_order[i]);

    int m = inorder_map[pre_order[i]];
    root->m_left = dfs(pre_order, inorder_map, i + 1, l, m - 1);
    root->m_right = dfs(pre_order, inorder_map, i + 1 + m - l, m + 1, r);

    return root;
}