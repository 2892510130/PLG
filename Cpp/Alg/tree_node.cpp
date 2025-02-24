#include <iostream>
#include <queue>
#include <stack>

#include "tree_node.h"

using namespace std;

void print_node_iter(TreeNode* node, TraversalOrder order) 
{
    stack<TreeNode*> s;
    TreeNode* tmp = node;
    TreeNode* pre_visited = nullptr;

    while (!s.empty() || tmp != nullptr)
    {
        if (tmp != nullptr)
        {
            s.push(tmp);
            if (order == TraversalOrder::DepthFirstPreOrder) cout << tmp->m_val << endl;
            tmp = tmp->m_left;
        }
        else
        {
            // TreeNode* pop_node = s.top();
            tmp = s.top();
            if (order == TraversalOrder::DepthFirstPostOrder)
            {
                if (tmp->m_right == nullptr || tmp->m_right == pre_visited)
                {
                    s.pop();
                    cout << tmp->m_val << endl;
                    pre_visited = tmp;
                    tmp = nullptr;
                }
                else
                {
                    tmp = tmp->m_right;
                }
            }
            else if (order == TraversalOrder::DepthFirstInOrder)
            {
                s.pop();
                cout << tmp->m_val << endl;
                tmp = tmp->m_right;
            }
            else if (order == TraversalOrder::DepthFirstPreOrder)
            {
                s.pop();
                tmp = tmp->m_right;
            }
        }
    }
}

void print_node(TreeNode* node, TraversalOrder order)
{
    if (order & (TraversalOrder::DepthFirstPreOrder | TraversalOrder::DepthFirstInOrder | TraversalOrder::DepthFirstPostOrder))
    {
        if (order == TraversalOrder::DepthFirstPreOrder) cout << node->m_val << endl;
    
        if (node->m_left != nullptr)
        {
            print_node(node->m_left, order);
        }

        if (order == TraversalOrder::DepthFirstInOrder) cout << node->m_val << endl;
        
        if (node->m_right != nullptr)
        {
            print_node(node->m_right, order);
        }

        if (order == TraversalOrder::DepthFirstPostOrder) cout << node->m_val << endl;
    }
    else if (order == TraversalOrder::BreadthFirst)
    {
        cout << "Printing in Breadth First order!" << endl;
        queue<TreeNode*> q;
        q.push(node);
        TreeNode* tmp;

        while (!q.empty())
        {
            tmp = q.front();
            q.pop();
            cout << tmp->m_val << endl;
            
            if (tmp->m_left != nullptr)
            {
                q.push(tmp->m_left);
            }

            if (tmp->m_right != nullptr)
            {
                q.push(tmp->m_right);
            }
        }

    }
}