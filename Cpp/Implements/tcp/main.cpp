#include <iostream>

#include "tcp_basic.hpp"

#define TEST(x) int x##family

struct Dog
{
    TEST(dog_);
};


int main() {
    Dog dog = { .dog_family = 5 };
    std::cout << "Hello, TCP connection\n";

    SocketAddress socket_test = SocketAddress::any(8080);
    std::cout << socket_test.length() << '\n';
}
