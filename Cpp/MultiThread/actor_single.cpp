#include <iostream>

#include "actor_single.h"

struct MsgClassC {
	std::string name;
	friend std::ostream& operator << (std::ostream& os, const MsgClassC& ca) {
		os << ca.name;
		return os;
	}
};

class ClassC : public ActorSingle<ClassC, MsgClassC> 
{
	friend class ActorSingle<ClassC, MsgClassC>;
public:
	~ClassC() {
		_bstop = true;
		_que.notify_stop();
		_thread.join();
		std::cout << "ClassC destruct " << std::endl;
	}

	void DealMsg(std::shared_ptr<MsgClassC> data) {
		std::cout << "class C deal msg is " << *data << std::endl;
	}
private:
	ClassC(){
		_thread = std::thread([this]() {
			for (; (_bstop.load() == false);) {
				std::shared_ptr<MsgClassC> data = _que.wait_and_pop();
				if (data == nullptr) {
					continue;
				}

				DealMsg(data);
			}

			std::cout << "ClassC thread exit " << std::endl;
        });
	}
};

struct MsgClassB {
	std::string name;
	friend std::ostream& operator << (std::ostream& os, const MsgClassB& ca) {
		os << ca.name;
		return os;
	}
};


class ClassB : public ActorSingle<ClassB, MsgClassB> {
	friend class ActorSingle<ClassB, MsgClassB>;
public:
	~ClassB() {
		_bstop = true;
		_que.notify_stop();
		_thread.join();
		std::cout << "ClassB destruct " << std::endl;
	}

	void DealMsg(std::shared_ptr<MsgClassB> data) {
		std::cout << "class B deal msg is " << *data << std::endl;

		MsgClassC msga;
		msga.name = "llfc";
		ClassC::Inst().PostMsg(msga);
	}
private:
	ClassB(){
		_thread = std::thread([this]() {
			for (; (_bstop.load() == false);) {
				std::shared_ptr<MsgClassB> data = _que.wait_and_pop();
				if (data == nullptr) {
					continue;
				}

				DealMsg(data);
			}

			std::cout << "ClassB thread exit " << std::endl;
		});
	}
};

struct MsgClassA {
	std::string name;
	friend std::ostream& operator << (std::ostream& os, const MsgClassA& ca) {
		os << ca.name;
		return os;
	}
};

class ClassA : public ActorSingle<ClassA, MsgClassA> {
	friend class ActorSingle<ClassA, MsgClassA>;
public:
	~ClassA() {
		_bstop = true;
		_que.notify_stop();
		_thread.join();
		std::cout << "ClassA destruct " << std::endl;
	}

	void DealMsg(std::shared_ptr<MsgClassA> data) {
		std::cout << "class A deal msg is " << *data << std::endl;

		MsgClassB msga;
		msga.name = "llfc";
		ClassB::Inst().PostMsg(msga);
	}
private:
	ClassA(){
		_thread = std::thread([this]() {
			for (; (_bstop.load() == false);) {
				std::shared_ptr<MsgClassA> data = _que.wait_and_pop();
				if (data == nullptr) {
					continue;
				}

				DealMsg(data);
			}

			std::cout << "ClassA thread exit " << std::endl;
			});
	}
};

int main()
{
    /*
        Actor Single: (threads notify each other)
        1. A process msg, then give another msg to B (another thread), divide by task. 
            So we can build a pipline (Producer-Consumer model), for example, video system : 
            one thread read, one process, one compress, one save.
        2. if we want A give msg to B, and B give msg to A, we need to
            2.1 only claim A in B, not include the head file (otherwise double include)
            2.2 in cpp inlucde both file
    */
    MsgClassA msga;
    msga.name = "llfc";
    ClassA::Inst().PostMsg(msga);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "main process exited!\n";
}