#include <iostream>
#include <vector>
#include <algorithm>

#include "heap.h" // we can use this to do heap sort, but we can also define a shiftdown seperate

using namespace std;

void selection_sort(vector<int> & v, bool reverse);
void bubble_sort(vector<int> & v, bool reverse);
void insertion_sort(vector<int> & v, bool reverse);
void quick_sort(vector<int> & v, bool reverse, int left, int right);
void quick_sort_2(vector<int> & v, bool reverse, int left, int right); // put elements both sides equally and deal with duplicated item
void quick_sort_3(vector<int> & v, bool reverse, int left, int right); // put items close to the center, decrease the stack size
int median(vector<int> & v, int left, int median, int right);
int partion(vector<int> & v, bool reverse, int left, int right);
int partion_2(vector<int> & v, bool reverse, int left, int right);
void merge(vector<int> & v, bool reverse, int left, int mid, int right);
void merge_sort(vector<int> & v, bool reverse, int left, int right);
void shift_down(vector<int> & v, bool reverse, int n, int i);
void heap_sort(vector<int> & v, bool reverse);
void bucket_sort(vector<int> & v, bool reverse);
void count_sort(vector<int> & v, bool reverse);
void print_vector(const vector<int> & vec);

int main()
{
    vector<int> v = {4, 1, 3, 1, 5, 2};
    cout << "Selection sort: " << endl;
    selection_sort(v, false);
    print_vector(v);
    selection_sort(v, true);
    print_vector(v);

    cout << "\nBubble sort: " << endl;
    bubble_sort(v, false);
    print_vector(v);
    bubble_sort(v, true);
    print_vector(v);

    cout << "\nInsertion sort: " << endl;
    insertion_sort(v, false);
    print_vector(v);
    insertion_sort(v, true);
    print_vector(v);

    cout << "\nQuick sort: " << endl;
    quick_sort(v, false, 0, v.size() - 1);
    print_vector(v);
    quick_sort(v, true, 0, v.size() - 1);
    print_vector(v);
    
    cout << "\nQuick sort 2: " << endl;
    quick_sort_2(v, false, 0, v.size() - 1);
    print_vector(v);
    quick_sort_2(v, true, 0, v.size() - 1);
    print_vector(v);

    cout << "\nQuick sort 3: " << endl;
    quick_sort_3(v, false, 0, v.size() - 1);
    print_vector(v);
    quick_sort_3(v, true, 0, v.size() - 1);
    print_vector(v);

    cout << "\nMerge sort: " << endl;
    merge_sort(v, false, 0, v.size() - 1);
    print_vector(v);
    merge_sort(v, true, 0, v.size() - 1);
    print_vector(v);

    // Heap* h_max = new Heap(v, false);
    // Heap* h_min = new Heap(v, true);
    cout << "\nHeap sort: " << endl;
    heap_sort(v, false);
    print_vector(v);
    heap_sort(v, true);
    print_vector(v);

    cout << "\nBucket sort: " << endl;
    bucket_sort(v, false);
    print_vector(v);
    bucket_sort(v, true);
    print_vector(v);

    cout << "\nCounting sort: " << endl;
    count_sort(v, false);
    print_vector(v);
    count_sort(v, true);
    print_vector(v);
}

void selection_sort(vector<int> & v, bool reverse)
{
    int n = v.size();

    for (int i = 0; i < n - 1; ++i)
    {
        int k = i;

        for (int j = i + 1; j < n; ++j)
        {
            if (!reverse && v[k] > v[j]) k = j;
            else if (reverse && v[k] < v[j]) k = j;
        }

        swap(v[i], v[k]);
    }
}

void bubble_sort(vector<int> & v, bool reverse)
{
    int n = v.size();

    for (int i = 0; i < n - 1; ++i) // for (int i = n - 1; i > 0; --i)
    {
        bool flag = false;
        for (int j = 1; j < n; ++j) // for (int j = 0; j < i; ++j)
        {
            if (!reverse && v[j] < v[j-1]) 
            {
                swap(v[j], v[j-1]);
                flag = true;
            }
            if (reverse && v[j] > v[j-1]) 
            {
                swap(v[j], v[j-1]);
                flag = true;
            }
        }

        if (!flag) break;
    }
}

void insertion_sort(vector<int> & v, bool reverse)
{
    int n = v.size();

    for (int i = 1; i < n; ++i)
    {
        int j = i - 1;
        int tmp = v[i];
        for (; j >= 0; --j)
        {
            if (!reverse)
            {
                if (v[j] <= tmp) break;
                else v[j+1] = v[j];
            }
            else
            {
                if (v[j] >= tmp) break;
                else v[j+1] = v[j];
            }
        }
        v[j+1] = tmp;
    }
}

void print_vector(const vector<int> & vec)
{
    for (const auto &v : vec)
    {
        cout << v << " | ";
    }
    cout << endl;
}

int median(vector<int> & v, int left, int median, int right)
{
    if ((v[left] <= v[median] && v[median] <= v[right]) || (v[right] <= v[median] && v[median] <= v[left]))
        return median;
    if ((v[median] <= v[left] && v[left] <= v[right]) || (v[right] <= v[left] && v[left] <= v[median]))
        return left;
    return right;
}

int partion(vector<int> & v, bool reverse, int left, int right)
{
    int i = left, j = right;

    // optimize the selected term, i.e the left, so we can sort (reversed) orderd list faster.
    // we can also random select
    int mid = median(v, left, (left + right) / 2, right);
    swap(v[left], v[mid]);
    while (true)
    {
        while (i < j && ((!reverse && v[j] >= v[left]) || (reverse && v[j] <= v[left])))
        {
            j--;
        }

        while (i < j && ((!reverse && v[i] <= v[left]) || (reverse && v[i] >= v[left])))
        {
            i++;
        }

        if (i >= j) break;
        
        swap(v[i], v[j]);
    }
    swap(v[i], v[left]);
    return i;
}

int partion_2(vector<int> & v, bool reverse, int left, int right)
{
    int i = left + 1, j = right;

    // optimize the selected term, i.e the left, so we can sort (reversed) orderd list faster.
    int mid = median(v, left, (left + right) / 2, right);
    swap(v[left], v[mid]);
    // int p = v[left]; // seems like we don't need this
    while (true)
    {
        while (i <= right && ((!reverse && v[i] < v[left]) || (reverse && v[i] > v[left])))
        {
            i++;
        }

        while (j > left && ((!reverse && v[j] > v[left]) || (reverse && v[j] < v[left])))
        {
            j--;
        }

        if (i > j) break;
        
        swap(v[i], v[j]);
        i++;
        j--;
    }
    swap(v[j], v[left]);
    return j;
}

void quick_sort_2(vector<int> & v, bool reverse, int left, int right)
{
    if (left >= right) return;
    int p = partion_2(v, reverse, left, right);
    quick_sort_2(v, reverse, left, p - 1);
    quick_sort_2(v, reverse, p + 1, right);

}

void quick_sort_3(vector<int> & v, bool reverse, int left, int right)
{
    if (left >= right) return;
    int mid = median(v, left, (left + right) / 2, right);
    swap(v[left], v[mid]);
    int p = v[left];
    int i = left, j = right + 1, k = left + 1;

    while (k < j)
    {
        if ((!reverse && v[k] < p) || (reverse && v[k] > p))
        {
            i++;
            swap(v[k], v[i]);
            k++;
        }
        else if (v[k] == p)
        {
            k++;
        }
        else
        {
            j--;
            swap(v[k], v[j]);
        }
    }
    swap(v[left], v[i]);
    quick_sort_3(v, reverse, left, i - 1); // we save a lot of the divide space here
    quick_sort_3(v, reverse, j, right); // we save a lot of the divide space here
}

void quick_sort(vector<int> & v, bool reverse, int left, int right)
{
    while (left < right)
    {
        int p = partion(v, reverse, left, right);

        if (p - left > right - p) // this for the optimize of the stack depth
        {
            quick_sort(v, reverse, left, p - 1);
            left = p + 1;
        }
        else
        {
            quick_sort(v, reverse, p + 1, right);
            right = p - 1;
        }

        // no stack optimize
        // quick_sort(v, reverse, left, p - 1);
        // quick_sort(v, reverse, p + 1, right);
    }
}

void merge(vector<int> & v, bool reverse, int left, int mid, int right)
{
    vector<int> tmp(right - left + 1);

    int i = left, j = mid + 1, k = 0;

    while (i <= mid && j <= right)
    {
        if ((!reverse && v[i] <= v[j]) || (reverse && v[i] > v[j])) tmp[k++] = v[i++];
        else tmp[k++] = v[j++];
    }

    while (i <= mid) tmp[k++] = v[i++];
    while (j <= right) tmp[k++] = v[j++];

    for (k = 0; k < tmp.size(); k++)
    {
        v[left + k] = tmp[k];
    }
}

void merge_sort(vector<int> & v, bool reverse, int left, int right)
{
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    merge_sort(v, reverse, left, mid);
    merge_sort(v, reverse, mid + 1, right);
    merge(v, reverse, left, mid, right);
}

void shift_down(vector<int> & v, bool reverse, int n, int i) // n is for the changed size of vec we want to heapify
{
    while (true) 
    {
        int l = 2 * i + 1, r = 2 * i + 2, index = i;
        if (reverse)
        {
            if (l < n)  index = v[l] <= v[index] ? l : index;
            if (r < n)  index = v[r] <= v[index] ? r : index;
        }
        else
        {
            if (l < n)  index = v[l] > v[index] ? l : index;
            if (r < n)  index = v[r] > v[index] ? r : index;
        }
        if (index == i) break;

        swap(v[index], v[i]);

        i = index;
    }
}

void heap_sort(vector<int> & v, bool reverse)
{
    for (int i = v.size() / 2 - 1; i >= 0; --i) // heapify all except the leaf node
    {
        shift_down(v, reverse, v.size(), i);

    }

    for (int i = v.size() - 1; i > 0; --i)
    {
        swap(v[0], v[i]);
        shift_down(v, reverse, i, 0);
    }
}

void bucket_sort(vector<int> & v, bool reverse) // not general!
{
    cout << "[Warning]: this alg is not general" << endl;
    int k = 3;
    vector<vector<int>> bucket(k);

    for (int i = 0; i < v.size(); ++i)
    {
        int index = (v[i] / 2);
        if (reverse) bucket[k - 1 - index].push_back(v[i]);
        else bucket[index].push_back(v[i]);
    }

    for (auto & item : bucket)
    {
        if (reverse) sort(item.begin(), item.end(), greater<int>());
        else sort(item.begin(), item.end());
    }

    int i = 0;
    for (auto & item : bucket)
    {
        for (auto & data : item) v[i++] = data;
    }
}

void count_sort(vector<int> & v, bool reverse)
{
    cout << "[Warning]: this alg is not general" << endl;
    int m = 0, n = v.size();
    for (int i = 0; i < n; ++i)
    {
        m = max(m, v[i]);
    }

    if (m < 0)
    {
        cout << "[Error]: only works for integer!" << endl;
        return;
    }

    vector<int> count(m + 1, 0); // do the count
    for (int i = 0; i < n; ++i) count[v[i]]++;

    for (int i = 1; i < m + 1; ++i) // transfert the count to prefix
    {
        count[i] += count[i-1];
    }

    vector<int> res(n);
    for (int i = n - 1; i >= 0; --i)
    {
        int num = v[i];
        if (reverse) res[n - count[num]] = num;
        else res[count[num] - 1] = num;
        count[num]--;
    }

    v = res;
}
