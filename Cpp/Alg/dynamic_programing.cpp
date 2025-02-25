#include <iostream>
#include <vector>

using namespace std;

int zero_one_packet(vector<int> & weight, vector<int> & value, int capacity);
int full_packet(vector<int> & weight, vector<int> & value, int capacity);

int main()
{
    vector<int> w = {10, 20, 30, 40, 50};
    // vector<int> w = {50, 40, 30, 20, 10};
    vector<int> v = {50, 120, 150, 210, 240};
    // vector<int> v = {210, 120, 150, 50, 240};
    int a = zero_one_packet(w, v, 30);
    cout << a << '\n';
    a = zero_one_packet(w, v, 70);
    cout << a << '\n';

    a = full_packet(w, v, 30);
    cout << a << '\n';
    a = full_packet(w, v, 70);
    cout << a << '\n';
}

int zero_one_packet(vector<int> & weight, vector<int> & value, int capacity)
{
    // optinal optimization:
    // 1. dp[i] only depends on dp[i-1], so we only need two c+1 sized vector
    // 2. if inner loop reverse, we can use only one c+1 sized vector, because dp[i] right only depends on dp[i-1] left
    int n = weight.size();
    vector<vector<int>> dp(n + 1, vector<int>(capacity + 1, 0));
    // dp[i][c] = max(dp[i-1][c], dp[i-1][c-w[i]] + w[i])

    for (int i = 1; i <= n; ++i)
    {
        for (int j = 1; j <= capacity; ++j)
        {
            if (j-weight[i-1] < 0) dp[i][j] = dp[i-1][j];
            else dp[i][j] = max(dp[i-1][j], dp[i-1][j-weight[i-1]] + value[i-1]);
            // cout << "dp[" << i << "][" << j << "]: " << dp[i][j] << '\n';
        }
    }

    return dp[n][capacity];
}

int full_packet(vector<int> & weight, vector<int> & value, int capacity)
{
    // optinal optimization:
    // 1. same as 0 - 1 problem
    // 2. forward inner loop, only 1 vector
    int n = weight.size();
    vector<vector<int>> dp(n + 1, vector<int>(capacity + 1, 0));
    // dp[i][c] = max(dp[i-1][c], dp[i-1][c-w[i]] + w[i])

    for (int i = 1; i <= n; ++i)
    {
        for (int j = 1; j <= capacity; ++j)
        {
            if (j-weight[i-1] < 0) dp[i][j] = dp[i-1][j];
            else dp[i][j] = max(dp[i-1][j], dp[i][j-weight[i-1]] + value[i-1]); // only diff
            // cout << "dp[" << i << "][" << j << "]: " << dp[i][j] << '\n';
        }
    }

    return dp[n][capacity];
}