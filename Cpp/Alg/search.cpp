#include <iostream>
#include <vector>
#include <random>
// #include <mt19937>

using namespace std;

int binary_search(const vector<int> & vec, int val);
int binary_search_lcro(const vector<int> & vec, int val); // l closed, r open
int binary_search_insert(const vector<int> & vec, int val);
int binary_search_left_edge(const vector<int> & vec, int val);
int binary_search_right_edge(const vector<int> & vec, int val);
int quick_select(vector<int>& nums, int left, int right, int k);
int partion(vector<int> & v, bool reverse, int left, int right);

int main()
{
    vector<int> a = {1, 3, 3, 3, 3, 7, 19, 28, 93, 115};
    cout << "Binary search:" << endl;
    int index = binary_search(a, 3);
    cout << index << endl;
    index = binary_search(a, 2);
    cout << index << endl;

    cout << "Binary search left closed right open way:" << endl;
    index = binary_search_lcro(a, 3);
    cout << index << endl;

    cout << "Binary search insert position:" << endl;
    index = binary_search_insert(a, 3);
    cout << index << endl;
    index = binary_search_insert(a, 2);
    cout << index << endl;

    cout << "Binary search left edge:" << endl;
    index = binary_search_left_edge(a, 3);
    cout << index << endl;
    index = binary_search_left_edge(a, 4);
    cout << index << endl;

    cout << "Binary search right edge:" << endl;
    index = binary_search_right_edge(a, 3);
    cout << index << endl;
    index = binary_search_right_edge(a, 4);
    cout << index << endl;

    vector<int> b = {3,2,3,1,2,4,5,5,6};
    quick_select(b, 0, b.size() - 1, 5);
}

int binary_search(const vector<int> & vec, int val)
{
    int left = 0, right = vec.size() - 1;

    while (left <= right)
    {
        int m = left + (right - left) / 2;

        if (vec[m] < val) left = m + 1;
        else if (vec[m] > val) right = m - 1;
        else return m;

    }

    return -1;
}

int binary_search_lcro(const vector<int> & vec, int val)
{
    int left = 0, right = vec.size();

    while (left <= right)
    {
        int m = left + (right - left) / 2;

        if (vec[m] < val) left = m + 1;
        else if (vec[m] > val) right = m;
        else return m;

    }

    return -1;
}

int binary_search_insert(const vector<int> & vec, int val)
{
    int left = 0, right = vec.size() - 1;

    while (left <= right)
    {
        int m = left + (right - left) / 2;

        if (vec[m] < val) left = m + 1;
        // else if (vec[m] > val) right = m - 1;
        // else return m;
        else right = m - 1; // This can handle the duplicated item in vec, if we want the leftest one
    }

    return left;
}

int binary_search_left_edge(const vector<int> & vec, int val)
{
    int index = binary_search_insert(vec, val);

    if (index == vec.size() || vec[index] != val) return -1; // index == vec.size() is needed, otherwise may cause out of index

    return index;
}

int binary_search_right_edge(const vector<int> & vec, int val) // or we can search for val - 0.5 for left edge, val + 0.5 for right edge
{
    int index = binary_search_insert(vec, val + 1) - 1; // this is clever, use the left edge index

    if (index == -1 || vec[index] != val) return -1;

    return index;
}

int quick_select(vector<int>& nums, int left, int right, int k)
{
    if (left >= right) return nums[k];

    int p = partion(nums, false, left, right);
    for (auto & i : nums) cout << i << " | ";
    cout << p << '\n';
    if (p > k) return quick_select(nums, left, p - 1, k);
    else if (p < k) return quick_select(nums, p + 1, right, k);
    else return nums[p];
}

int partion(vector<int> & v, bool reverse, int left, int right)
{
    int i = left, j = right;
    mt19937 gen(0);
    uniform_int_distribution<int> dist(left, right);
    int mid = dist(gen);
    swap(v[left], v[mid]);
    int p = v[left];
    while (i < j)
    {
        while (i < j && ((!reverse && v[j] >= p) || (reverse && v[j] <= p)))
        {
            j--;
        }

        while (i < j && ((!reverse && v[i] <= p) || (reverse && v[i] >= p)))
        {
            i++;
        }

        // if (i >= j) break;
        
        swap(v[i], v[j]);
    }
    swap(v[i], v[left]);
    return i;
}