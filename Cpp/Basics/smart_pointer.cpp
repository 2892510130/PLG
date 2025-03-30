#include <iostream>
#include <memory>

int main()
{
    std::shared_ptr<int> p1(new int(5));
    std::shared_ptr<int> p2 = p1;
    p2.reset(new int(6));
    std::cout << *p1 << " " << *p2 << '\n';
    std::cout << p1.use_count() << " " << p2.use_count() << '\n';
}