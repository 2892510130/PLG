#include <iostream>
#include <stdexcept>

class Complex {
private:
    double real;
    double imag;

public:
    // 构造函数
    Complex(double r = 0.0, double i = 0.0) : real(r), imag(i) {}

    // 重载 << 运算符（友元函数）
    friend std::ostream& operator<<(std::ostream& os, const Complex& c);

    // 重载 >> 运算符（友元函数）
    friend std::istream& operator>>(std::istream& is, Complex& c);
};

// 实现 << 运算符
std::ostream& operator<<(std::ostream& os, const Complex& c) {
    os << "(" << c.real;
    if (c.imag >= 0)
        os << " + " << c.imag << "i)";
    else
        os << " - " << -c.imag << "i)";
    return os;
}

// 实现 >> 运算符
std::istream& operator>>(std::istream& is, Complex& c) {
    // 假设输入格式为：real imag
    is >> c.real >> c.imag;
    return is;
}

// 示例
int main() {
    Complex c1;
    std::cout << "Please give two number: ";
    std::cin >> c1;
    std::cout << "Your input is: " << c1 << std::endl;
    return 0;
}
