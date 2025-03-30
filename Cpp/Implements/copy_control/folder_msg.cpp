#include "folder_msg.h"
#include <iostream>

using namespace std;

//将Message保存在指定Folder中
void Message::save(Folder &f)
{
    //将文件夹f添加到msg的folders里
    this->folders.insert(&f);
    //将本msg添加到folder中
    f.addMsg(*this);
}
//从Folder中删除Message
void Message::remove(Folder &f)
{
    //将文件夹从msg的folders里删除
    this->folders.erase(&f);
    f.remMsg(*this);
}

//保存指定的msg
void Folder::addMsg(Message &msg)
{
    this->msgs.insert(make_pair(msg.contents, &msg));
}
//移除指定的msg
void Folder::remMsg(Message &msg)
{
    if (this->msgs.count(msg.contents)) this->msgs.erase(msg.contents);
}

//将本Message添加到参数msg的folder中
void Message::add_to_Folders(const Message &msg)
{
    for (auto f : msg.folders)
    {
        f->addMsg(*this);
    }
}

//拷贝构造函数将m的folders拷贝给自己
Message::Message(const Message &m)
{
    contents = m.contents;
    folders = m.folders;
    add_to_Folders(m);
}

//从folders中的每个Folder删除本Message
void Message::remove_from_Folders()
{
    for (auto f : folders)
    {
        f->remMsg(*this);
    }
}

//析构函数
Message::~Message()
{
    remove_from_Folders();
}

// 拷贝赋值运算符
Message &Message::operator=(const Message &msg)
{
    remove_from_Folders();
    contents = msg.contents;
    folders = msg.folders;
    add_to_Folders(msg);
    return *this;
}

void Message::move_Folders(Message *m)
{
    //将m的Folders交接给本对象
    this->folders = m->folders;
    //将本对象和folders关联
    for (auto f : folders)
    {
        //解除folders和m的关联
        f->remMsg(*m);
        //添加本对象和folders的关联
        f->addMsg(*this);
    }
    //清除m的folders
    m->folders.clear();
}

//利用move_Folders函数实现移动构造函数
Message::Message(Message &&m) : contents(std::move(m.contents))
{
    move_Folders(&m);
}

Message &Message::operator=(Message &&m)
{
    if (this != &m)
    {
        remove_from_Folders();
        contents = std::move(m.contents);
        move_Folders(&m);
    }
    return *this;
}

void Message::printMsg()
{
    cout << "contents is " << contents << endl;
}

void swap(Message &lhs, Message &rhs)
{
    //将lhs从关联的folders中移除
    lhs.remove_from_Folders();
    //将rhs从关联的folders中移除
    rhs.remove_from_Folders();
    //交换两个成员
    swap(lhs.contents, rhs.contents);
    swap(lhs.folders, rhs.folders);
    //重新将lhs添加到关联的folders中
    lhs.add_to_Folders(lhs);
    //重新将rhs添加到关联的folders中
    rhs.add_to_Folders(rhs);
}

Folder::Folder(const Folder &f)
{
    this->msgs = f.msgs;
    this->name = f.name;
    add_msgs(f);
}

//删除folder中的所有msgs
void Folder::remove_msgs()
{
    for (auto msgit = this->msgs.begin(); msgit != this->msgs.end();)
    {
        msgit->second->folders.erase(this);
        msgit = this->msgs.erase(msgit);
    }
}
// 将f中的msgs添加到本folder中
void Folder::add_msgs(const Folder &f)
{
    //将f的msgs添加到本folder
    for (auto mp : f.msgs)
    {
        //将消息保存在本folder
        mp.second->save(*this);
    }
}

//析构函数
Folder::~Folder()
{
    remove_msgs();
    msgs.clear();
}

//拷贝赋值运算符
Folder &Folder::operator=(const Folder &f)
{
    // 先从本folder的msg解除和本folder的关联
    remove_msgs();
    //再将f的参数赋值给本folder
    this->name = f.name;
    this->msgs = f.msgs;
    //将msgs添加到本folder
    add_msgs(f);
    return *this;
}

void swap(Folder &lf, Folder &rf)
{
    //先解除lf的msg和其的联系
    lf.remove_msgs();
    //再解除rf的msg和其的联系
    rf.remove_msgs();
    //交换数据结构
    swap(lf.msgs, rf.msgs);
    swap(lf.name, rf.name);
    //最后各自绑定msg和其folder的联系
    lf.add_msgs(lf);
    rf.add_msgs(rf);
}

void test_msgfolder()
{
    auto f1 = new Folder("folder1");
    auto msg1 = new Message("msg1");
    msg1->save(*f1);
    auto msg2 = new Message(*msg1);
    delete msg1;
    delete f1;
    msg2->printMsg();
    delete msg2;

    cout << "Program Finished!\n";
    // auto f2 = new Folder("folder2");
    // auto msg2 = new Message("msg2");
    // msg2->save(*f2);
    // auto f3 = new Folder(*f1);
    // *f3 = *f2;
    // swap(f1, f2);
    // delete (f1);
    // delete (msg1);
    // delete (f2);
    // delete (msg2);
    // delete (f3);
    // auto msg4 = new Message("msg4");
    // msg4->printMsg();
    // auto msg3 = new Message("msg3");
    // msg3->printMsg();
    // *msg4 = std::move(*msg3);
    // msg4->printMsg();
    // delete msg4;
    // delete msg3;
}

int main()
{
    test_msgfolder();
}