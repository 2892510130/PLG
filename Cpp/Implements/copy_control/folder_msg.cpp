#include "folder_msg.h"
#include <iostream>

using namespace std;

//��Message������ָ��Folder��
void Message::save(Folder &f)
{
    //���ļ���f��ӵ�msg��folders��
    this->folders.insert(&f);
    //����msg��ӵ�folder��
    f.addMsg(*this);
}
//��Folder��ɾ��Message
void Message::remove(Folder &f)
{
    //���ļ��д�msg��folders��ɾ��
    this->folders.erase(&f);
    f.remMsg(*this);
}

//����ָ����msg
void Folder::addMsg(Message &msg)
{
    this->msgs.insert(make_pair(msg.contents, &msg));
}
//�Ƴ�ָ����msg
void Folder::remMsg(Message &msg)
{
    if (this->msgs.count(msg.contents)) this->msgs.erase(msg.contents);
}

//����Message��ӵ�����msg��folder��
void Message::add_to_Folders(const Message &msg)
{
    for (auto f : msg.folders)
    {
        f->addMsg(*this);
    }
}

//�������캯����m��folders�������Լ�
Message::Message(const Message &m)
{
    contents = m.contents;
    folders = m.folders;
    add_to_Folders(m);
}

//��folders�е�ÿ��Folderɾ����Message
void Message::remove_from_Folders()
{
    for (auto f : folders)
    {
        f->remMsg(*this);
    }
}

//��������
Message::~Message()
{
    remove_from_Folders();
}

// ������ֵ�����
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
    //��m��Folders���Ӹ�������
    this->folders = m->folders;
    //���������folders����
    for (auto f : folders)
    {
        //���folders��m�Ĺ���
        f->remMsg(*m);
        //��ӱ������folders�Ĺ���
        f->addMsg(*this);
    }
    //���m��folders
    m->folders.clear();
}

//����move_Folders����ʵ���ƶ����캯��
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
    //��lhs�ӹ�����folders���Ƴ�
    lhs.remove_from_Folders();
    //��rhs�ӹ�����folders���Ƴ�
    rhs.remove_from_Folders();
    //����������Ա
    swap(lhs.contents, rhs.contents);
    swap(lhs.folders, rhs.folders);
    //���½�lhs��ӵ�������folders��
    lhs.add_to_Folders(lhs);
    //���½�rhs��ӵ�������folders��
    rhs.add_to_Folders(rhs);
}

Folder::Folder(const Folder &f)
{
    this->msgs = f.msgs;
    this->name = f.name;
    add_msgs(f);
}

//ɾ��folder�е�����msgs
void Folder::remove_msgs()
{
    for (auto msgit = this->msgs.begin(); msgit != this->msgs.end();)
    {
        msgit->second->folders.erase(this);
        msgit = this->msgs.erase(msgit);
    }
}
// ��f�е�msgs��ӵ���folder��
void Folder::add_msgs(const Folder &f)
{
    //��f��msgs��ӵ���folder
    for (auto mp : f.msgs)
    {
        //����Ϣ�����ڱ�folder
        mp.second->save(*this);
    }
}

//��������
Folder::~Folder()
{
    remove_msgs();
    msgs.clear();
}

//������ֵ�����
Folder &Folder::operator=(const Folder &f)
{
    // �ȴӱ�folder��msg����ͱ�folder�Ĺ���
    remove_msgs();
    //�ٽ�f�Ĳ�����ֵ����folder
    this->name = f.name;
    this->msgs = f.msgs;
    //��msgs��ӵ���folder
    add_msgs(f);
    return *this;
}

void swap(Folder &lf, Folder &rf)
{
    //�Ƚ��lf��msg�������ϵ
    lf.remove_msgs();
    //�ٽ��rf��msg�������ϵ
    rf.remove_msgs();
    //�������ݽṹ
    swap(lf.msgs, rf.msgs);
    swap(lf.name, rf.name);
    //�����԰�msg����folder����ϵ
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