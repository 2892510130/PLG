#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>

using namespace std;

void use_istreamiter()
{
    //输入流迭代器
    istream_iterator<int> in_int(cin);
    //迭代器终止标记
    istream_iterator<int> in_eof;
    vector<int> in_vec;
    while (in_int != in_eof)
    {
        in_vec.push_back(*in_int++);
    }
    for_each(in_vec.begin(), in_vec.end(), [](const int &i)
             { cout << i << " "; });
    cout << endl;
}

int main()
{
    // std::vector<int> vec;
    
    // // 从 std::cin 读取整数，直到输入结束（Ctrl+D 或 Ctrl+Z）
    // std::cout << "Please enter: \n";
    // std::copy(std::istream_iterator<int>(std::cin),  // 开始
    //           std::istream_iterator<int>(),          // 结束
    //           std::back_inserter(vec));             // 目标

    // // 输出结果
    // std::cout << "You input:";
    // for (int n : vec) {
    //     std::cout << n << " ";
    // }
    // std::cout << std::endl;

    use_istreamiter();

    return 0;
}