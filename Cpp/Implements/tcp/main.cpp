#include <iostream>
#include <netinet/in.h>

#define TEST(x) int x##family

struct Dog
{
    TEST(dog_);
};


int main() {
    Dog dog = { .dog_family = 5 };
    std::cout << "Hello, TCP connection\n";

    int number = 0x1234;
    int number_little = ntohl(number);
    int number_big = htonl(number);
    std::cout << number << ", " << number_little << ", " << number_big << '\n';

    char tmp[256];
    std::cout << sizeof(tmp) << '\n';
}
