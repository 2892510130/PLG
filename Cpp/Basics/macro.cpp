#include <iostream>

using namespace std;

#define exp(s) printf("test s is:%s\n",s)
#define exp1(s) printf("test s is:%s\n",#s)
#define exp2(s) #s 
#define expA(s) printf("ǰ׺���Ϻ���ַ���Ϊ:%s\n",gc_##s)  //gc_s�������
#define expB(s) printf("ǰ׺���Ϻ���ַ���Ϊ:%s\n",gc_  ##  s)  //gc_s�������
#define gc_hello1 "I am gc_hello1"

int main() {
    exp("hello");
    exp1(hello);

    string str = exp2(   bac );
    cout<<str<<" "<<str.size()<<endl;
    /**
     * ���Դ��������ǰ��ͺ���Ŀո�
     */
    string str1 = exp2( asda  bac );
    /**
     * ���������������ڿո�ʱ�������������Զ����Ӹ������ַ�����
     * ��ÿ�����ַ���֮����һ���ո����ӣ�����ʣ��ո�
     */
    cout<<str1<<" "<<str1.size()<<endl;

    // ## combine the marco
    const char * gc_hello = "I am gc_hello";
    expA(hello);
    expB(hello1);
    return 0;
}