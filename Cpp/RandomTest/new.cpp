#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <string>
#include <map>

using namespace std;

class Solution {
    map<int, int> _map;
public:
    int deleteAndEarn(vector<int>& nums) {
        for (int number : nums)
        {
            // -_map[number]++;
            _map[number] += number;
        }

        vector<int> dp(_map.size());
        int i = 0, prev;
        for (const auto &[k, v] : _map)
        {
            if (i == 0)
            {
                dp[i] = v;
            }
            else if (i == 1) 
            {
                if (k == prev + 1) dp[i] = max(dp[0], v);
                else dp[i] = dp[0] + v;
            }
            else 
            {
                if (k == prev + 1) dp[i] = max(dp[i-1], dp[i-2] + v);
                else dp[i] = dp[i-1] + v;
            }
            i++;
            prev = k;
        }

        return dp.back();
    }
};

int main()
{

    Solution s;
    // vector<int> a;
    // // auto has_loop = s.maxProfit(a);
    // vector<vector<int>> grid = {{1,1,1,1,0,0,0,0},{1,1,1,1,0,0,0,0},{1,1,1,1,1,1,1,1},{1,1,1,1,1,1,1,1},{1,1,1,1,0,0,0,0},{1,1,1,1,0,0,0,0},{1,1,1,1,0,0,0,0},{1,1,1,1,0,0,0,0}};
    // auto k = s.construct(grid);
    // cout << k << '\n';
    vector<int> a = {2,2,3,3,3,4};
    vector<int> b = {3, 1};
    auto c = s.deleteAndEarn(b);
    cout << c << '\n';
}
