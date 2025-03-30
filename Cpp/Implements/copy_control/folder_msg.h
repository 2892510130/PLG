#ifndef __MSGFOLDER_H__
#define __MSGFOLDER_H__
#include <string>
#include <set>
#include <unordered_map>
using namespace std;
extern void test_msgfolder();
class Folder;
class Message
{
    friend class Folder;

public:
    // folder被隐式初始化为空集合
    explicit Message(const std::string &str = "") : contents(str) {}
    // 拷贝控制成员，用来管理指向本Message的指针
    Message(const Message &);
    // 拷贝赋值运算符
    Message &operator=(const Message &);
    //析构函数
    ~Message();
    Message(Message &&m);
    Message &operator=(Message &&m);
    //将Message保存在指定Folder中
    void save(Folder &);
    //从Folder中删除Message
    void remove(Folder &);
    bool operator<(const Message *msg)
    {
        return this->contents < msg->contents;
    }

    friend void swap(Message &lhs, Message &rhs);
    //将m的Folders交接给本类对象
    //并且实现Folders和本类对象的关联
    //接触Folders和m的关联
    void move_Folders(Message *m);
    //打印内容
    void printMsg();

private:
    //  消息内容
    std::string contents;
    // 消息所属文件夹
    std::set<Folder *> folders;
    //将本Message添加到参数msg的folder中
    void add_to_Folders(const Message &msg);
    //从folders中的每个Folder删除本Message
    void remove_from_Folders();
};
class Folder
{
    friend class Message;

public:
    explicit Folder(const std::string &nm = "") : name(nm) {}
    //拷贝控制成员
    Folder(const Folder &);
    //拷贝赋值运算符
    Folder &operator=(const Folder &);
    //析构函数
    ~Folder();
    //保存指定的msg
    void addMsg(Message &);
    //移除指定的msg
    void remMsg(Message &);
    bool operator<(const Folder *folder)
    {
        return this->name < folder->name;
    }

    friend void swap(Folder &lf, Folder &rf);

private:
    //删除folder中的所有msgs
    void remove_msgs();
    // 将f中的msgs添加到本folder中
    void add_msgs(const Folder &f);
    //文件夹名字
    std::string name;
    //包含的消息列表
    std::unordered_map<std::string, Message *> msgs;
};
#endif