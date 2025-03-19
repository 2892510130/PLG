#include <iostream>

using namespace std;

enum class Color : char
{
    BLUE = 'b',
    Red = 'r'
};

int main()
{
    Color c = Color::Red;
    cout << static_cast<char>(c) << '\n';
}