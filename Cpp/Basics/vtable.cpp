/*
    vtable and vptr
*/

#include <iostream>
// #include <stdio.h>

using namespace std;

typedef void (*Fun)();

class Base // ������vtable�е�˳�������˳�����
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
    void* vptr_addr = (void *)*(unsigned long *)obj;  //64λ����ϵͳ��8�ֽڣ�ͨ��*(unsigned long *)objȡ��8�ֽڣ���vptrָ��
    printf("vptr_addr:%p\n",vptr_addr);

    /**
     * @brief ͨ��vptrָ�����virtual table����Ϊ�����ÿ��Ԫ��64λ����������8���ֽڣ����ͨ��*(unsigned long *)vptr_addrȡ��8�ֽ�
     * ���ǣ��������������������ҵĵ�����MinGw gcc ����16���ֽڲ���
     */
    void* func_addr = (void *)*((unsigned long *)vptr_addr + offset * 2);
    printf("func_addr:%p\n",func_addr);
    return (Fun)func_addr;
}

int main(void)
{
    Base ptr;
    Derived d;
    Base *pt = new Derived(); // ����ָ��ָ��������
    Base &pp = ptr; // ��������ָ�����
    Base &p = d; // ��������ָ��������
    cout<<"�������ֱ�ӵ���"<<endl;
    ptr.fun1();
    cout<<"��������ָ�����ʵ��"<<endl;
    pp.fun1(); 
    cout<<"����ָ��ָ��������ʵ���������麯��?"<<endl;
    pt->fun1();
    cout<<"��������ָ��������ʵ���������麯��?"<<endl;
    p.fun1();

    cout << sizeof(unsigned long *) << '\n';

    // �ֶ�����vptr��vtable
    Fun f1 = getAddr(pt, 0);
    (*f1)();
    Fun f2 = getAddr(pt, 1);
    (*f2)();
    Fun f3 = getAddr(pt, 2);
    (*f3)();
    delete pt;
    return 0;
}