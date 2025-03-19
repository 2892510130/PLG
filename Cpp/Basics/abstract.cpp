#include <iostream>
using namespace std;

class Base {
public:
    Base() { cout << "Constructor: Base" << endl; }
    Base(int i_) : i(i_) { cout << "Constructor: Base" << endl; }
    // virtual ~Base() { cout << "Destructor : Base" << endl; }
    ~Base() { cout << "Destructor : Base" << endl; } // will not call the derived destructor!!!

    Base& operator=(const Base &other) 
    {
        cout << "Base assign\n";
        i = other.i;
    };

    int i;
};

class Derived : public Base {
    // int num[5];
public:
    Derived() { cout << "Constructor: Derived" << endl; }
    Derived(int j_, int i_) : Base(i_), j(j_) { cout << "Constructor: Derived" << endl; }
    ~Derived() { cout << "Destructor : Derived" << endl; }

    Derived& operator=(const Derived &other) 
    {
        cout << "Derived assign\n";
        j = other.j;
        i = other.i;
    };

    int j;
};

class Base2 {
    public:
      virtual void fun() { cout << "Base Fun\n"; }
      //   friend int main();
};
    
class Derived2 : public Base2 {
private:
    void fun() { cout << "Derived Fun\n"; }
};
    
int main() {
    Base *Var = new Derived(); // always call base con -> derive con -> derive de -> base de || if the base is virtual de
    // Derived *Var = new Derived();
    delete Var;

    Base2 *ptr = new Derived2;
    ptr->fun();

    cout << "--------- assign order: \n";
    Derived d(1, 2);
    Derived d2(3, 4);
    d2 = d;
    cout << d2.i << " " << d2.j << '\n';
    return 0;
}