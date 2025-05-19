#include <iostream>

#include "actor_single.h"
#include "parallel_alg.h"

int main()
{
    /*
        How to use the data structure do the task divide
        1. parallel algorithm
        2. divide based on task type : 
            2.1 use CRTP (Curiously recurring template pattern, 
            derive from a template which has a paramater that is itself)
        3. divide method 1 : divide data to threads
           divide method 2 : divide recursivly
           divide method 3 : divide task
    */
    std::list<int> a = {6, 3, 5, 6, 7, 9, 2, 1};
    auto b = parallel_quick_sort(a);
    for (auto &data : b)
    {
        std::cout << data << " | ";
    }
    std::cout << '\n';
}