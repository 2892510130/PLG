/* 
    Static in C++
    1. static variable : stay in the whole memory, and init just once
        1.1  static var in a class : see below, static var in a class, then this 
        obj will be global one, it will be destructed after the main function ends.   
        1.2  And, the static var should be initialized by user.
    2. static obj.
*/

#include<iostream> 

using namespace std; 

class Fruit
{
public:
    static int count;
    int value = 11;

    Fruit() 
    {
        cout << "Construct the obj\n";
    }

    ~Fruit()
    {
        cout << "Destruct the obj\n";
    }
};

class Apple : public Fruit
{

};

int Fruit::count = 1; // 1.2 must initialize by the user

int main()
{
    Fruit f;
    Apple a;
    cout << "End the main function\n"; // 1.1 a and f become global static obj
    f.count = 2;
    a.count = 2;
    cout << Fruit::count << '\n';
    cout << a.count << '\n';

    static Apple static_a;
}