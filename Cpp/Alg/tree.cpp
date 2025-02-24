/* Traversal methods in tree
   in recursion and iteration way. Breadth first and depth first (pre, in, post order) search. */
#include <iostream>

#include "tree_node.h"

using namespace std;

void insert_node();
void delete_node();

int main() 
{
    TreeNode* n0 = new TreeNode(0);  
    TreeNode* n1 = new TreeNode(1);  
    TreeNode* n2 = new TreeNode(2);  
    TreeNode* n3 = new TreeNode(3);  
    TreeNode* n4 = new TreeNode(4);
    TreeNode* n5 = new TreeNode(5);
    TreeNode* n6 = new TreeNode(6);

    n0->m_left = n1;
    n0->m_right = n2;
    n1->m_left = n3;
    n1->m_right = n4;
    n2->m_left = n5;
    n2->m_right = n6;

    print_node(n0, TraversalOrder::BreadthFirst);
    cout << "Printing in Depth First pre order!" << endl;
    print_node(n0, TraversalOrder::DepthFirstPreOrder);
    cout << "Printing in Depth First in order!" << endl;
    print_node(n0, TraversalOrder::DepthFirstInOrder);
    cout << "Printing in Depth First post order!" << endl;
    print_node(n0, TraversalOrder::DepthFirstPostOrder);
    
    cout << "Test iteration on pre order" << endl;
    print_node_iter(n0, TraversalOrder::DepthFirstPreOrder);
    cout << "Test iteration on in order" << endl;
    print_node_iter(n0, TraversalOrder::DepthFirstInOrder);
    cout << "Test iteration on post order" << endl;
    print_node_iter(n0, TraversalOrder::DepthFirstPostOrder);

}
