#include <iostream>
#include <mutex>
#include <thread>
#include <set>

template<typename T>
class ThreadSafeList
{
private:
    struct node
    {
        std::mutex m;
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;

        node() : next() {}
        node(T const& value) : data(std::make_shared<T>(value)) {}
    };

    node m_head;

public:
    ThreadSafeList() {}
    ~ThreadSafeList() 
    {
        remove_if([](node const&) {return true;});
    }

    ThreadSafeList(ThreadSafeList const& other) = delete;
    ThreadSafeList& operator=(ThreadSafeList const& other) = delete;

    template<typename Predicate>
	void remove_if(Predicate p)
	{
		node* current = &m_head;
		std::unique_lock<std::mutex> lk(m_head.m);
		while (node* const next = current->next.get())
		{
			std::unique_lock<std::mutex> next_lk(next->m);
			if (p(*next->data)) // if it satisfies the predicate, then delete the data, or change current to next
			{
				std::unique_ptr<node> old_next = std::move(current->next);
				current->next = std::move(next->next);
				next_lk.unlock();
			}
			else
			{
				lk.unlock();
				current = next;
				lk = std::move(next_lk);
			}
		}
	}

    template<typename Predicate>
    std::shared_ptr<T> find_first_if(Predicate p)
    {
        node* current = &m_head;
        std::unique_lock<std::mutex> lk(m_head.m);
        while (node* const next = current->next.get())
        {
            std::unique_lock<std::mutex> next_lk(next->m);
            lk.unlock();
            if (p(*next->data))
            {
                return next->data;
            }
            current = next;
            lk = std::move(next_lk);
        }
        return std::shared_ptr<T>();
    }

    template<typename Function>
    void for_each(Function f)
    {
        node* current = &m_head;
        std::unique_lock<std::mutex> lk(m_head.m);
        while (node* const next = current->next.get())
        {
            std::unique_lock<std::mutex> next_lk(next->m);
            lk.unlock();
            f(*next->data);
            current = next;
            lk = std::move(next_lk);
        }
    }

    void push_front(T const& value)
    {
        std::unique_ptr<node> new_node(new node(value));
        std::lock_guard<std::mutex> lk(m_head.m);
        new_node->next = std::move(m_head.next);
        m_head.next = std::move(new_node);
    }
};

class MyClass
{
public:
    MyClass(int i):_data(i){}

    friend std::ostream& operator << (std::ostream& os, const MyClass& mc){
        os << mc._data;
        return os;
    }

    int GetData() const
	{
		return _data;
	}

private:
    int _data;
};


void TestThreadSafeList()
{
    std::set<int> removeSet;
    ThreadSafeList<MyClass> thread_safe_list;
    std::thread t1([&]()
    {
        for(int i = 0; i < 10; i++)
        {
            MyClass mc(i);
            thread_safe_list.push_front(mc);
        }
        
    });


    std::thread t2([&]()
    {
        for (int i = 0; i < 10; )
        {
            
            auto find_res = thread_safe_list.find_first_if([&](auto & mc) {
                return mc.GetData() == i;
            });

            if(find_res == nullptr)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            removeSet.insert(i);
            i++;
        }
    });

    t1.join();
    t2.join();

    std::cout << "size of list: " << removeSet.size() << '\n';
}

template<typename T>
class ThreadSafeDoublePushList
{
	struct node_d
	{
		std::mutex m;
		std::shared_ptr<T> data;
		std::unique_ptr<node_d> next;
		node_d() :
			next()
		{}
		node_d(T const& value) :
			data(std::make_shared<T>(value))
		{}
	};

	node_d head;
	node_d *  last_node_ptr;
	//控制最后一个节点指针更新
	std::mutex   last_ptr_mtx;
public:
	ThreadSafeDoublePushList()
	{
		last_node_ptr = &head;
	}

	~ThreadSafeDoublePushList()
	{
		remove_if([](node_d const&) {return true; });
	}

	ThreadSafeDoublePushList(ThreadSafeDoublePushList const& other) = delete;
	ThreadSafeDoublePushList& operator=(ThreadSafeDoublePushList const& other) = delete;

	void push_front(T const& value)
	{
		std::unique_ptr<node_d> new_node(new node_d(value));
		std::lock_guard<std::mutex> lk(head.m);
		new_node->next = std::move(head.next);
		head.next = std::move(new_node);
		//更新最后一个节点
		if (head.next->next == nullptr) {
			std::lock_guard<std::mutex> last_lk(last_ptr_mtx);
			last_node_ptr = head.next.get();
		}
	}

	void push_back(T const& value) {
		//防止于push_head同时进行
		//并且保证头部或者删除节点更新last_node_ptr唯一, 所以同时加锁
		std::unique_ptr<node_d> new_node(new node_d(value));
		std::lock(last_node_ptr->m, last_ptr_mtx);
		std::unique_lock<std::mutex> lk(last_node_ptr->m, std::adopt_lock);
		std::unique_lock<std::mutex> last_lk(last_ptr_mtx, std::adopt_lock);
		//原来的最后节点的下一个节点指向新生成的节点
		last_node_ptr->next = std::move(new_node);
		//将最后一个节点后移
		last_node_ptr = last_node_ptr->next.get();
	}

	template<typename Function>
	void for_each(Function f)
	{
		node_d* current = &head;
		std::unique_lock<std::mutex> lk(head.m);
		while (node_d* const next = current->next.get())
		{
			std::unique_lock<std::mutex> next_lk(next->m);
			lk.unlock();
			f(*next->data);
			current = next;
			lk = std::move(next_lk);
		}
	}

	template<typename Predicate>
	std::shared_ptr<T> find_first_if(Predicate p)
	{
		node_d* current = &head;
		std::unique_lock<std::mutex> lk(head.m);
		while (node_d* const next = current->next.get())
		{
			std::unique_lock<std::mutex> next_lk(next->m);
			lk.unlock();
			if (p(*next->data))
			{
				return next->data;
			}
			current = next;
			lk = std::move(next_lk);
		}
		return std::shared_ptr<T>();
	}

	template<typename Predicate>
	void remove_if(Predicate p)
	{
		node_d* current = &head;
		std::unique_lock<std::mutex> lk(head.m);
		while (node_d* const next = current->next.get())
		{
			std::unique_lock<std::mutex> next_lk(next->m);
			if (p(*next->data))
			{
				std::unique_ptr<node_d> old_next = std::move(current->next);
				current->next = std::move(next->next);
				//判断删除的是否为最后一个节点
				if (current->next == nullptr) {
					std::lock_guard<std::mutex> last_lk(last_ptr_mtx);
					last_node_ptr = current;
				}
				next_lk.unlock();
			}
			else
			{
				lk.unlock();
				current = next;
				lk = std::move(next_lk);
			}
		}
	}

	template<typename Predicate>
	bool remove_first(Predicate p)
	{
		node_d* current = &head;
		std::unique_lock<std::mutex> lk(head.m);
		while (node_d* const next = current->next.get())
		{
			std::unique_lock<std::mutex> next_lk(next->m);
			if (p(*next->data))
			{
				std::unique_ptr<node_d> old_next = std::move(current->next);
				current->next = std::move(next->next);
				//判断删除的是否为最后一个节点
				if (current->next == nullptr) {
					std::lock_guard<std::mutex> last_lk(last_ptr_mtx);
					last_node_ptr = current;
				}
				next_lk.unlock();

				return true;
			}

			lk.unlock();
			current = next;
			lk = std::move(next_lk);
		}

		return false;
	}

	//从断言处查询满足条件的第一个点之前插入
	template<typename Predicate>
	void insert_if(Predicate p, T const & value)
	{
		node_d* current = &head;
		std::unique_lock<std::mutex> lk(head.m);
		while(node_d * const next = current->next.get())
		{
			std::unique_lock<std::mutex> next_lk(next->m);
			if(p(*(next->data)))
			{
				std::unique_ptr<node_d> new_node(new node_d(value));

				//缓存当前节点的下一个节点作为旧节点
				auto old_next = std::move(current->next);

				//新节点指向当前节点的下一个节点
				new_node->next = std::move(old_next);
				//当前节点的下一个节点更新为新节点
				current->next = std::move(new_node);
				return;
			}
			lk.unlock();
			current = next;
			lk = std::move(next_lk);
		}
	}
};


int main()
{
    /*
     *  Safe List:
     *  1. fine grained control of the mutex, so every node has it's own mutex
     *  2. first lock head, then lock next, then... iteratly forward to find
     * */
    TestThreadSafeList();
}
