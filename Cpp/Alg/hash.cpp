#include <iostream>
#include <unordered_map>
#include <vector>
#include <utility>

using namespace std;

int main() 
{
    unordered_map<int, double> mp;
    mp[1] = 1.1;
    mp[2] = 2.2;

    for (const auto &[k, v] : mp)
    {
        cout << k << ": " << v << endl;
    }

    int hash = 0;
    char c = 'a';
    hash = ((hash << 4) ^ (hash >> 28) ^ (int)c);
    cout << hash << endl;
}
