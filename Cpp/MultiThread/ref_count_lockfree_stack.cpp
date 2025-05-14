#include <cassert>
#include <memory>
#include <atomic>
#include <set>
#include <mutex>
#include <iostream>
#include <thread>

template<typename T>
class RefCountStack {
private:
	//前置声明节点类型
	struct count_node;
	struct counted_node_ptr {
		//1 外部引用计数
		int external_count;
		//2 节点地址
		count_node* ptr;
	};

	struct count_node {
		//3  数据域智能指针
		std::shared_ptr<T> data;
		//4  节点内部引用计数
		std::atomic<int>  internal_count;
		//5  下一个节点
		counted_node_ptr  next;
		count_node(T const& data_): data(std::make_shared<T>(data_)), internal_count(0) {}
	};
	//6 头部节点
	std::atomic<counted_node_ptr> head;
    std::atomic<int> test_delete_ptr{0};

public:
	//增加头部节点引用数量
	void increase_head_count(counted_node_ptr& old_counter) {
		counted_node_ptr new_counter;
		
		do {
			new_counter = old_counter;
			++new_counter.external_count;
		}//7  循环判断保证head和old_counter想等时做更新,多线程情况保证引用计数原子递增。
		while (!head.compare_exchange_strong(old_counter, new_counter, 
			std::memory_order_acquire, std::memory_order_relaxed));
		//8  走到此处说明head的external_count已经被更新了
		old_counter.external_count = new_counter.external_count;
	}

	std::shared_ptr<T> pop() {
		counted_node_ptr old_head = head.load();
		for (;;) {
			increase_head_count(old_head);
			count_node* const ptr = old_head.ptr;
			//1  判断为空责直接返回
			if (!ptr) {
				return std::shared_ptr<T>();
			}

			//2 本线程如果抢先完成head的更新, compare_exchange_strong is atomic, so head will always be new, increase_head_count make sure
            // if old_head is head, then the data in head is new.
			if (head.compare_exchange_strong(old_head, ptr->next, std::memory_order_relaxed)) {
				//返回头部数据
				std::shared_ptr<T> res;
				//交换数据
				res.swap(ptr->data);
				//3 减少外部引用计数，先统计到目前为止增加了多少外部引用
				int const count_increase = old_head.external_count - 2;
				//4 将内部引用计数添加
				if (ptr->internal_count.fetch_add(count_increase, std::memory_order_release) == -count_increase) {
					delete ptr;
                    test_delete_ptr.fetch_add(1);
                    std::cout << "delete ptr " << test_delete_ptr.load() << '\n';
				}
				return res;
			} else if (ptr->internal_count.fetch_add(-1, std::memory_order_acquire) == 1) { //5
				//如果当前线程操作的head节点已经被别的线程更新，则减少内部引用计数
				//当前线程减少内部引用计数，返回之前值为1说明指针仅被当前线程引用

                // if use relaxed above, we can use this to make sure the swap finished
				// ptr->internal_count.load(std::memory_order_acquire);
                delete ptr;
                test_delete_ptr.fetch_add(1);
                std::cout << "delete ptr " << test_delete_ptr.load() << '\n';
                return std::shared_ptr<T>();
			}
		}
	}

	RefCountStack(){
		counted_node_ptr head_node_ptr;
		//头节点开始只做标识用，没效果
		head_node_ptr.external_count = 0;
		head_node_ptr.ptr = nullptr;
		head.store(head_node_ptr);
	}

	~RefCountStack() {
		//循环出栈
		while (pop());
	}

	void push(T const& data) {
		counted_node_ptr  new_node;
		new_node.ptr = new count_node(data);
		new_node.external_count = 1;
		new_node.ptr->next = head.load();
		while (!head.compare_exchange_weak(new_node.ptr->next, new_node, 
			std::memory_order_release, std::memory_order_relaxed)); // make sure new node can be seen.
	}	
};

void TestSingleRefStack() {
    RefCountStack<int>  single_ref_stack;
    std::set<int>  rmv_set;
    std::mutex set_mtx;

    std::thread t1([&]() {
        for (int i = 0; i < 20; i++) {
            single_ref_stack.push(i);
            std::cout << "push data " << i << " success!" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        });

    std::thread t2([&]() {
        for (int i = 0; i < 10;) {
            auto head = single_ref_stack.pop();
            if (!head) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            std::lock_guard<std::mutex> lock(set_mtx);
            rmv_set.insert(*head);
            std::cout << "pop data " << *head << " success!" << std::endl;
            i++;
        }
        });

    std::thread t3([&]() {
        for (int i = 0; i < 10;) {
            auto head = single_ref_stack.pop();
            if (!head) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            std::lock_guard<std::mutex> lock(set_mtx);
            rmv_set.insert(*head);
            std::cout << "pop data " << *head << " success!" << std::endl;
            i++;
        }
        });

    t1.join();
    t2.join();
    t3.join();

    assert(rmv_set.size() == 20);
}

int main()
{
    /*
     *   Ref Count Lock-free Stack:
     *   1. g++ .\ref_count_lockfree_stack.cpp -o ref_count_lockfree_stack -Wall -Wextra -latomic [must have this],
     *      the atomic of the struct is with one int and one pointer, which takes 128 bits. And this need atomic lib.
     *   2. in atomic store the struct, not the pointer. As we do not want different threads work on same memory, in 
     *      this way, we create copy, and will not effect other thread.
     *   3. use two counter, external_count count the head (counted_node_ptr) threads, and internal_count count the 
     *      count_node. Only one will have problem, see the comment below, the code with problem.
     *   4. some explaination: t1 with head{p1, external_count=1}, we get to 1 before 2, with head{p1, external_count=2},
     *      then t2 get in push one node, head{p2, external_count=1}, for t1 old_head{p1, external_count=2}, compare in 2
     *      will not match, so go to else if, p1->internal_count = -1, we do not delete the p1. Then another thread pop 
     *      p2 successfully. Then t3 came with old_head{p1, external_count=3} (origin 2 does not decrease), it get into 2
     *      swap the data, internal_count = 3 - 2 = 1. p1->internal_count = -1 == -internal_count, so delete the ptr. No 
     *      memory leak!
     * */
    TestSingleRefStack();
}

/*
 std::shared_ptr<T> pop() {
    //0 处
    ref_node* old_head = head.load();
    for (;;) {
        //1 只要执行pop就对引用计数+1并更新到head中
        ref_node* new_head;
        do {
            new_head = old_head;
            //7 处
            new_head->_ref_count += 1;
        } while (!head.compare_exchange_weak(old_head, new_head));
        //4 
        old_head = new_head;

        auto* node_ptr = old_head->_node_ptr;
        if (node_ptr == nullptr) {
            return  std::shared_ptr<T>();
        }

        //2 比较head和old_head想等则交换否则说明head已经被其他线程更新
        if (head.compare_exchange_strong(old_head, node_ptr->_next)) {
                
            //要返回的值
            std::shared_ptr<T> res;
            //交换智能指针
            //5 处
            res.swap(node_ptr->_data);

            //6 增加的数量
            int increase_count = old_head->_ref_count.fetch_sub(2);
                
            //3 处判断仅有当前线程持有指针则删除
            if (increase_count == 2) {
                delete node_ptr;
            }

            return res;
        }else {
            if (old_head->_ref_count.fetch_sub(1) == 1) {
                delete node_ptr;
            }
        }
    }
}
 * */
