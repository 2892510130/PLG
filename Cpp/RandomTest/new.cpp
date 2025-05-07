#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <string>
#include <map>
#include <stack>
#include <algorithm>
#include <functional>

using namespace std;

struct TreeNode {
    int val;
    string name;
    TreeNode *left;
    TreeNode *right;
    TreeNode() : val(0), left(nullptr), right(nullptr) {}
    TreeNode(int x) : val(x), name(""), left(nullptr), right(nullptr) {}
    TreeNode(int x, string n) : val(x), name(n), left(nullptr), right(nullptr) {}
    TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), name(""), left(left), right(right) {}
};

template <typename T>
void print_vec(vector<T> &vec)
{
    for (T v : vec)
    {
        cout << v << " | ";
    }
    cout << '\n';
}

/**
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     TreeNode *left;
 *     TreeNode *right;
 *     TreeNode(int x) : val(x), left(NULL), right(NULL) {}
 * };
 */
class Solution {
    public:
        int hIndex(vector<int>& citations) {
            int n = citations.size(), tot = 0;
            vector<int> counter(n + 1);
            for (int i = 0; i < n; i++) {
                if (citations[i] >= n) {
                    counter[n]++;
                } else {
                    counter[citations[i]]++;
                }
            }
            print_vec(counter);
            for (int i = n; i >= 0; i--) {
                tot += counter[i];
                if (tot >= i) {
                    return i;
                }
            }
            return 0;
        }
};

int main()
{

    Solution s;
    // TreeNode* b = new TreeNode(1, "b");
    // TreeNode* b1 = new TreeNode(2, "b1");
    // TreeNode* b2 = new TreeNode(3, "b2");
    // TreeNode* b3 = new TreeNode(-1, "b3");
    // TreeNode* b4 = new TreeNode(3, "b4");
    // TreeNode* b5 = new TreeNode(-3, "b5");
    // TreeNode* b6 = new TreeNode(-2, "b6");
    // b->left = b1;
    // b->right = b2;
    // b1->left = b2;
    // b2->left = b3;
    // b1->right = b4;
    // b->right = b5;
    // b5->left = b6;

    vector<int> a = {1, 3, 1, 2, 2};
    // string a = "hit";
    // string b = "cog";
    // vector<string> list = {"hot","dot","dog","lot","log","cog"};

    auto c = s.hIndex(a);
    // cout << c << '\n';
}
