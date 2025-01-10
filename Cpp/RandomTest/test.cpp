#include <iostream>
#include <deque>

int main() 
{
    std::deque<double> queue;
    // queue.resize(6, 0.0);

    queue.push_back(1.0);
    queue.pop_front();
    queue.push_back(1.0);
    queue.pop_front();

    for (const auto & item : queue)
    {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}