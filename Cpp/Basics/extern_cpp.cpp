// first compile it : gcc -c .\extern.c
// then link it : g++ .\extern_cpp.cpp .\extern.o

#include <iostream>
using namespace std;
extern "C" {
    #include "extern.h"
}
int main() {
    add(2,3);
    return 0;
}