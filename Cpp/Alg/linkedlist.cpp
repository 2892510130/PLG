#include <iostream>

using namespace std;

struct LinkedList 
{
    int var;
    LinkedList* next;
    LinkedList(int var): var(var), next(nullptr) {};
};

int main(int argc, char* argv[])
{
    LinkedList* n0 = new LinkedList(1);
    LinkedList* n1 = new LinkedList(2);
    n0->next = n1;
    cout << n0->next->var << endl;
}
