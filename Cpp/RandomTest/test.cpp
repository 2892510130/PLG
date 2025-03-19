#include <iostream>
#include <deque>
#include <vector>
#include <string>
#include <array>
#include <algorithm>
#include <unordered_map>

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

struct Node
{
    int val;
    Node* next;
    Node(int v) : val(v), next(nullptr) {};
};

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
    
    std::vector<std::vector<int>> vv = {{1, 2}, {0, 3}};
    sort(vv.begin(), vv.end(), [](auto &x, auto &y){return x[0] < y[0];});
    for (auto & v : vv)
    {
        for (auto & num : v)
        {
            std::cout << num << " ";
        }
        std::cout << '\n';
    }

    std::vector<std::string> str_vec;
    std::string ss = std::string("Hello");
    str_vec.push_back(std::move(ss));
    ss = std::string("Wo");
    std::cout << ss << "___" << str_vec[0] << '\n';

    Node* n1, *n2;
    n1 = new Node(3);
    n2 = new Node(5);
    n1->next = n2;
    n2->next = n1;
    std::cout << n2->val << '\n';

    std::vector<int> vec2(3, 0);
    vec2.emplace_back(5);
    std::cout << vec2.size() << '\n';

    std::vector<std::vector<int>> a11 = {{1, 10}, {3,4}};
    auto max_one = std::max_element(a11.begin(), a11.end());
    auto max_two = std::max_element((*max_one).begin(), (*max_one).end());
    std::cout << *max_two << '\n';
}
