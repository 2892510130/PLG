#include <condition_variable>
#include <mutex>
#include <queue>
#include <atomic>
#include <memory>
#include <iostream>
#include <thread>

//最大风险指针数量
unsigned const max_hazard_pointers = 100;

//风险指针
struct hazard_pointer {
	std::atomic<std::thread::id> id;
	std::atomic<void*> pointer;
};

//风险指针数组
hazard_pointer hazard_pointers[max_hazard_pointers];

//风险指针持有类
class HPOwner {
public:
	HPOwner(HPOwner const&) = delete;
	HPOwner operator=(HPOwner const&) = delete;
	HPOwner():hp(nullptr){
		bind_hazard_pointer();
	}

	std::atomic<void*>& get_pointer() {
		return hp->pointer;
	}

    void get_id()
    {
        std::cout << hp->id.load() << '\n';
    }

	~HPOwner() {
		hp->pointer.store(nullptr);
		hp->id.store(std::thread::id());
	}
private:
	void bind_hazard_pointer() {
		for (unsigned i = 0; i < max_hazard_pointers; ++i) {
			std::thread::id old_id;
			if (hazard_pointers[i].id.compare_exchange_strong(old_id, std::this_thread::get_id())) {
				hp = &hazard_pointers[i];
				break;
			}
		}

		if (!hp) {
			throw std::runtime_error("No hazard pointers available");
		}
	}
	hazard_pointer* hp;
};

std::atomic<void*>& get_hazard_pointer_for_current_thread() {
	//每个线程都具有自己的风险指针 线程本地变量, only first time call this func will construct it for this thread.
	thread_local static HPOwner hazzard;  
	return hazzard.get_pointer();
}

template<typename T>
class HazardPointerStack {
private:
	
	//栈节点
	struct node {
		std::shared_ptr<T> data;
		node* next;
		node(T const& data_) :data(std::make_shared<T>(data_)) {}
	};

	//待删节点
	struct data_to_reclaim {
		node* data;
		data_to_reclaim* next;
		data_to_reclaim(node * p):data(p), next(nullptr){}
		~data_to_reclaim() {
			delete data;
		}
	};

	HazardPointerStack(const HazardPointerStack&) = delete;
	HazardPointerStack& operator = (const HazardPointerStack&) = delete;

	std::atomic<node*> head;
	std::atomic<data_to_reclaim*> nodes_to_reclaim;
public:
	HazardPointerStack() {}

	void push(T const& data) {
		node* const new_node = new node(data);    //⇽-- - 2			
		new_node->next = head.load();    //⇽-- - 3			
		while (!head.compare_exchange_weak(new_node->next, new_node));    //⇽-- - 4	
	}

	bool outstanding_hazard_pointers_for(void* p)
	{
		for (unsigned i = 0; i < max_hazard_pointers; ++i)
		{
			if (hazard_pointers[i].pointer.load() == p)
			{
				return true;
			}
		}
		return false;
	}

	void add_to_reclaim_list(data_to_reclaim* reclaim_node) {
		reclaim_node->next = nodes_to_reclaim.load();
		while (!nodes_to_reclaim.compare_exchange_weak(reclaim_node->next, reclaim_node));
	}

	void reclaim_later(node* old_head) {
		add_to_reclaim_list(new data_to_reclaim(old_head));
	}

	void delete_nodes_with_no_hazards() {
		data_to_reclaim* current = nodes_to_reclaim.exchange(nullptr);
		while (current) {
			data_to_reclaim* const next = current->next;
			if (!outstanding_hazard_pointers_for(current->data)) {
				delete current;
			}
			else {
				add_to_reclaim_list(current);
			}

			current = next;
		}
	}

	std::shared_ptr<T> pop() {
		//1 从风险列表中获取一个节点给当前线程
		std::atomic<void*>& hp = get_hazard_pointer_for_current_thread();
		node* old_head = head.load();
		do
		{
			node* temp;
			do
			{
				temp = old_head;
				hp.store(old_head);
				old_head = head.load();
			}//2 如果old_head和temp不等说明head被其他线程更新了，需重试
			while (old_head != temp);
		}//3 将当前head更新为old_head->next，如不满足则重试
		while (old_head &&
			!head.compare_exchange_strong(old_head, old_head->next));
		// 4一旦更新了head指针，便将风险指针清零
		hp.store(nullptr);
		std::shared_ptr<T> res;
		if (old_head)
		{
			res.swap(old_head->data);
			//5 删除旧有的头节点之前，先核查它是否正被风险指针所指涉
			if (outstanding_hazard_pointers_for(old_head))
			{
				//6 延迟删除
				reclaim_later(old_head);
			}
			else
			{
				//7 删除头部节点
				delete old_head;
			}
			//8 删除没有风险的节点
			delete_nodes_with_no_hazards();
		}
		return res;
	}

};

int main()
{
    /*
     *  Hazard Pointer:
     *  1. in no lock stack, we mentioned if always we have multiple threads pop, the node should be reclaimed 
     *     may never reclaim, it's length will keep growing.
     *  2. so our approach is to let the last thread that pop do the reclaim, it's like GC. Each therad has it's 
     *     own hazard pointer, other threads can not delete it.
     *  3. but we know GC like is slow! We have to walk through the hazard_pointer for outstanding_hazard_pointers_for.
     *     we can use some space trade for time tech, like create 2N hazard_pointer, only for n > N we do this walk.
     * */

    thread_local static HPOwner hazzard;
    hazzard.get_id();
}
