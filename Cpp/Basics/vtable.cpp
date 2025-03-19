/*
    vtable and vptr
*/

#include <iostream>
// #include <stdio.h>

using namespace std;

typedef void (*Fun)();

class Base // 函数在vtable中的顺序跟声明顺序相关
{
    public:
        Base(){};

        virtual void fun1()
        {
            cout << "Base::fun1()" << endl;
        }

        virtual void fun3()
        {
            cout << "Base::fun3()" << endl;
        }

        virtual void fun2()
        {
            cout << "Base::fun2()" << endl;
        }

        ~Base(){};
};

class Derived: public Base
{
    public:
        Derived(){};

        void fun2()
        {
            cout << "DerivedClass::fun2()" << endl;
        }

        void fun1()
        {
            cout << "Derived::fun1()" << endl;
        }

        ~Derived(){};
};

Fun getAddr(void* obj, unsigned int offset)
{
    cout << "=======================\n";
    void* vptr_addr = (void *)*(unsigned long *)obj;  //64位操作系统，8字节，通过*(unsigned long *)obj取出8字节，即vptr指针
    printf("vptr_addr:%p\n",vptr_addr);

    /**
     * @brief 通过vptr指针访问virtual table，因为虚表中每个元素64位编译器下是8个字节，因此通过*(unsigned long *)vptr_addr取出8字节
     * 但是！！！！！！！！！！我的电脑上MinGw gcc 必须16个字节才行
     */
    void* func_addr = (void *)*((unsigned long *)vptr_addr + offset * 2);
    printf("func_addr:%p\n",func_addr);
    return (Fun)func_addr;
}

int main(void)
{
    Base ptr;
    Derived d;
    Base *pt = new Derived(); // 基类指针指向派生类
    Base &pp = ptr; // 基类引用指向基类
    Base &p = d; // 基类引用指向派生类
    cout<<"基类对象直接调用"<<endl;
    ptr.fun1();
    cout<<"基类引用指向基类实例"<<endl;
    pp.fun1(); 
    cout<<"基类指针指向派生类实例并调用虚函数?"<<endl;
    pt->fun1();
    cout<<"基类引用指向派生类实例并调用虚函数?"<<endl;
    p.fun1();

    cout << sizeof(unsigned long *) << '\n';

    // 手动查找vptr和vtable
    Fun f1 = getAddr(pt, 0);
    (*f1)();
    Fun f2 = getAddr(pt, 1);
    (*f2)();
    Fun f3 = getAddr(pt, 2);
    (*f3)();
    delete pt;
    return 0;
}