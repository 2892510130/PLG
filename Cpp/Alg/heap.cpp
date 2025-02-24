/* Heap impl in vector */

#include <iostream>
#include <vector>

#include "heap.h"

using namespace std;


int Heap::left(int i)
{
    return 2 * i + 1;
}

int Heap::right(int i)
{
    return 2 * i + 2;
}

int Heap::parent(int i)
{
    return (i - 1) / 2;
}

int Heap::peek()
{
    return m_heap[0];
}

Heap::Heap(const vector<int> &vec, bool min_heap)
{
    m_heap = vec;
    m_min_heap = min_heap;
    heapify();
}

void Heap::shift_up(int i)
{
    while (true)
    {
        int p = parent(i);

        if (p < 0 || ((!m_min_heap && m_heap[p] >= m_heap[i]) || (m_min_heap && m_heap[p] < m_heap[i])))
            break;

        swap(m_heap[p], m_heap[i]);

        i = p;
    }
}

void Heap::shift_down(int i)
{
    while (true) 
    {
        int l = left(i), r = right(i), index = i;
        if (m_min_heap)
        {
            if (l < m_heap.size())  index = m_heap[l] <= m_heap[index] ? l : index;
            if (r < m_heap.size())  index = m_heap[r] <= m_heap[index] ? r : index;
        }
        else
        {
            if (l < m_heap.size())  index = m_heap[l] > m_heap[index] ? l : index;
            if (r < m_heap.size())  index = m_heap[r] > m_heap[index] ? r : index;
        }
        if (index == i) break;

        swap(m_heap[index], m_heap[i]);

        i = index;
    }
}

void Heap::push(int val)
{
    m_heap.push_back(val);
    shift_up(m_heap.size() - 1);
}

void Heap::pop()
{
    if (m_heap.size() == 0) return;

    swap(m_heap[0], m_heap[m_heap.size() - 1]);
    m_heap.pop_back();
    shift_down(0);
}

void Heap::heapify()
{
    for (int i = parent(m_heap.size() - 1); i >= 0; i--)
    {
        shift_down(i);
    }
}


Heap* create_heap()
{
    vector<int> vec = {9, 8, 6, 6, 7, 5, 2, 1, 4, 3, 6, 2};
    Heap* h = new Heap(vec, false);
    return h;
}

void print_vec(const vector<int> &vec)
{
    for (const auto &v : vec)
    {
        cout << v << " | ";
    }
    cout << endl;
}

int main()
{
    Heap* h = create_heap();
    print_vec(h->m_heap);
    h->push(7);
    print_vec(h->m_heap);
    h->pop();
    print_vec(h->m_heap);

    cout << "max heap: " << endl;
    vector<int> vec = {1, 4, 3, 7, 5, 2, 6, 2, 9, 8, 6, 6};
    Heap* h2 = new Heap(vec, false);
    h2->heapify();
    print_vec(h2->m_heap);

    cout << "min heap: " << endl;
    vector<int> vec2 = {1, 4, 3, 7, 5, 2, 6, 2, 9, 8, 6, 6};
    Heap* h3 = new Heap(vec, true);
    h3->heapify();
    print_vec(h3->m_heap);
}