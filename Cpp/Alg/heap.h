#include <vector>

class Heap
{
    public:
        int left(int i);

        int right(int i);

        int parent(int i);

        int peek();

        Heap(const std::vector<int> &vec, bool min_heap);

        void shift_up(int i);

        void shift_down(int i);

        void push(int val);

        void pop();

        void heapify();

        std::vector<int> m_heap;
        bool m_min_heap;
};