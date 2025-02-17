#include <iostream>
#include <deque>
#include <vector>
#include <string>

void ref_param(std::string & s1, std::string & s2)
{
    std::string temp = s1;
    s1 = s2;
    s2 = temp;
}

void value_param(std::string s1, std::string s2)
{
    std::string temp = s1;
    s1 = s2;
    s2 = temp;
}

/* This one has 2 problem:
 * 1. s1 and s2 are temp value, will not change the pointer in the main.
 * 2. temp is a temp value in the function. will leak memory.*/
void point_param(std::string * s1, std::string * s2)
{
	std::string temp = *s1;
	s1 = s2;
	s2 = &temp;
}

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

    std::string s1 = "Hello";
    std::string s2 = "World";
    value_param(s1, s2);
    std::cout << s1 << ", " << s2 << std::endl;
    ref_param(s1, s2);
    std::cout << s1 << ", " << s2 << std::endl;

	std::vector<double> vec(2);
	std::cout << vec[0] << ", " << vec[1] << std::endl;

	int a = 5, b = 4;
	std::cout << (a & 1) << ", " << (b & 1) << std::endl;
}
