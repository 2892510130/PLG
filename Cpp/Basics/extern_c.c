// first compile it : g++ -c .\extern_c.cpp
// then link it : gcc .\extern_c.c .\extern_c.o

extern int add(int x,int y);
int main() {
    add(2,3);
    return 0;
}