#include<iostream>
#include<cstring>

using namespace std;

class Person{
public:
    typedef enum {
        BOY = 0, 
        GIRL 
    }SexType;

    Person(char *n, int a,SexType s){
        name=new char[strlen(n)+1];
        strcpy(name,n);
        age=a;
        sex=s;
    }

    int get_age() const{

        return this->age; 
    }

    Person& add_age(int a){
        age+=a;
        return *this; 
    }

    ~Person(){
        cout << "deleting " << sex << '\n';
        delete [] name;
    }

private:
    char * name;
    int age;
    SexType sex;
};


int main(){
    Person p("zhangsan",20,Person::BOY); 
    Person l("lili",10,Person::GIRL); 
    cout<<p.get_age()<<endl;
    cout<<p.add_age(10).get_age()<<endl;
    return 0;
}