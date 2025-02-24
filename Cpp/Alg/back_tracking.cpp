#include <iostream>
#include <vector>
#include <algorithm>

#include "tree_node.h"

using namespace std;

void find_path_to_val(TreeNode* root, int target, vector<TreeNode*> &path, vector<vector<TreeNode*>> &res, int ignore_val = INT_MAX); // preorder
void print_vector(const vector<TreeNode*> & vec);

int main()
{
    TreeNode* n0 = new TreeNode(0);  
    TreeNode* n1 = new TreeNode(1);  
    TreeNode* n2 = new TreeNode(2);  
    TreeNode* n3 = new TreeNode(3);  
    TreeNode* n4 = new TreeNode(4);
    TreeNode* n5 = new TreeNode(3);
    TreeNode* n6 = new TreeNode(6);
    TreeNode* n7 = new TreeNode(3);

    n0->m_left = n1;
    n0->m_right = n2;
    n1->m_left = n3;
    n1->m_right = n4;
    n2->m_left = n5;
    n2->m_right = n6;
    n3->m_left = n7;

    vector<TreeNode*> path;
    vector<vector<TreeNode*>> res;
    find_path_to_val(n0, 3, path, res);
    cout << "Find path: " << endl;
    for (auto &p : res) print_vector(p);

    path.clear();
    res.clear();
    find_path_to_val(n0, 3, path, res, 1);
    cout << "\nFind path with clip: " << endl;
    for (auto &p : res) print_vector(p);
}

void find_path_to_val(TreeNode* root, int target, vector<TreeNode*> &path, vector<vector<TreeNode*>> &res, int ignore_val)
{
    if (root == nullptr || root->m_val == ignore_val) return;

    path.push_back(root);

    if (root->m_val == target) res.push_back(path); // record

    find_path_to_val(root->m_left, target, path, res, ignore_val);
    find_path_to_val(root->m_right, target, path, res, ignore_val);

    path.pop_back(); // retreat
}

// for no duplicated num in choices, and same num can be selected multiple times
void backtrack_subset_sum(vector<int> &state, int target, vector<int> &choices, int start, vector<vector<int>> &res) 
{
    // 子集和等于 target 时，记录解
    if (target == 0) {
        res.push_back(state);
        return;
    }
    // 遍历所有选择
    // 剪枝二：从 start 开始遍历，避免生成重复子集
    for (int i = start; i < choices.size(); i++) {
        // 剪枝一：若子集和超过 target ，则直接结束循环
        // 这是因为数组已排序，后边元素更大，子集和一定超过 target
        if (target - choices[i] < 0) {
            break;
        }
        // 尝试：做出选择，更新 target, start
        state.push_back(choices[i]);
        // 进行下一轮选择
        backtrack_subset_sum(state, target - choices[i], choices, i, res);
        // 回退：撤销选择，恢复到之前的状态
        state.pop_back();
    }
}

// for duplicated num in choices, and same num can only be selected 1 time
void backtrack_subset_sum_with_duplicated_num(vector<int> &state, int target, vector<int> &choices, int start, vector<vector<int>> &res) {
    // 子集和等于 target 时，记录解
    if (target == 0) {
        res.push_back(state);
        return;
    }
    // 遍历所有选择
    // 剪枝二：从 start 开始遍历，避免生成重复子集
    // 剪枝三：从 start 开始遍历，避免重复选择同一元素，应该放在下面的下一轮选择的位置？
    for (int i = start; i < choices.size(); i++) {
        // 剪枝一：若子集和超过 target ，则直接结束循环
        // 这是因为数组已排序，后边元素更大，子集和一定超过 target
        if (target - choices[i] < 0) {
            break;
        }
        // 剪枝四：如果该元素与左边元素相等，说明该搜索分支重复，直接跳过
        if (i > start && choices[i] == choices[i - 1]) {
            continue;
        }
        // 尝试：做出选择，更新 target, start
        state.push_back(choices[i]);
        // 进行下一轮选择，注意此处是i+1，区别于无重复元素多次选择的情况
        backtrack_subset_sum_with_duplicated_num(state, target - choices[i], choices, i + 1, res);
        // 回退：撤销选择，恢复到之前的状态
        state.pop_back();
    }
}

vector<vector<int>> subsetSumI(vector<int> &nums, int target) {
    vector<int> state;              // 状态（子集）
    sort(nums.begin(), nums.end()); // 对 nums 进行排序
    int start = 0;                  // 遍历起始点
    vector<vector<int>> res;        // 结果列表（子集列表）
    backtrack_subset_sum(state, target, nums, start, res);
    return res;
}

void print_vector(const vector<TreeNode*> & vec)
{
    for (const auto &v : vec)
    {
        cout << v->m_val << " | ";
    }
    cout << endl;
}

/* 判断当前状态是否为解 */
bool isSolution(vector<TreeNode *> &state) {
    return !state.empty() && state.back()->m_val == 7;
}

/* 记录解 */
void recordSolution(vector<TreeNode *> &state, vector<vector<TreeNode *>> &res) {
    res.push_back(state);
}

/* 判断在当前状态下，该选择是否合法 */
bool isValid(vector<TreeNode *> &state, TreeNode *choice) {
    return choice != nullptr && choice->m_val != 3;
}

/* 更新状态 */
void makeChoice(vector<TreeNode *> &state, TreeNode *choice) {
    state.push_back(choice);
}

/* 恢复状态 */
void undoChoice(vector<TreeNode *> &state, TreeNode *choice) {
    state.pop_back();
}

/* 回溯算法：例题三 */
void backtrack(vector<TreeNode *> &state, vector<TreeNode *> &choices, vector<vector<TreeNode *>> &res) {
    // 检查是否为解
    if (isSolution(state)) {
        // 记录解
        recordSolution(state, res);
        // return; // the framework not needed here
    }
    // 遍历所有选择
    for (TreeNode *choice : choices) {
        // 剪枝：检查选择是否合法
        if (isValid(state, choice)) {
            // 尝试：做出选择，更新状态
            makeChoice(state, choice);
            // 进行下一轮选择
            vector<TreeNode *> nextChoices{choice->m_left, choice->m_right};
            backtrack(state, nextChoices, res);
            // 回退：撤销选择，恢复到之前的状态
            undoChoice(state, choice);
        }
    }
}