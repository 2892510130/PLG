#include <algorithm>
#include <iostream>
#include <memory>
#include <shared_mutex>
#include <thread>
#include <list>
#include <mutex>
#include <vector>
#include <map>
#include <iterator>
#include <set>

template<typename Key, typename Value, typename Hash = std::hash<Key>>
class ThreadSafeLookupTable
{
    private:
        class BuckedType
        {
        private:
            friend class ThreadSafeLookupTable;

            typedef std::pair<Key, Value> bucket_value;
            typedef std::list<bucket_value> bucket_data;
            typedef typename bucket_data::iterator bucket_iterator;

            bucket_data data;
            mutable std::shared_mutex mutex;

            bucket_iterator find_entry_for(const Key &key)
            {
                return std::find_if(data.begin(), data.end(), [&](bucket_value const &item) {
                    return item.first == key;
                });
            }

        public:
            Value value_for(Key const &key, Value const &default_value)
            {
                std::shared_lock<std::shared_mutex> lock(mutex);
			    bucket_iterator const found_entry = find_entry_for(key);
			    return (found_entry == data.end()) ? default_value : found_entry->second;
            }

            void add_or_update_mapping(Key const& key, Value const& value)
		    {
			    std::unique_lock<std::shared_mutex> lock(mutex);
		    	bucket_iterator const found_entry = find_entry_for(key);
			    if (found_entry == data.end())
			    {
				    data.push_back(bucket_value(key, value));
			    }
			    else
			    {
			    	found_entry->second = value;
			    }
		    }

            void remove_mapping(Key const& key)
		    {
			    std::unique_lock<std::shared_mutex> lock(mutex);
			    bucket_iterator const found_entry = find_entry_for(key);
			    if (found_entry != data.end())
			    {
			    	data.erase(found_entry);
			    }
		    }
        };

        std::vector<std::unique_ptr<BuckedType>> m_buckets;
        Hash m_hasher;

        BuckedType& get_bucket(Key const& key) const
	    {
		    std::size_t const bucket_index = m_hasher(key) % m_buckets.size();
		    return *m_buckets[bucket_index];
	    }

    public:
        ThreadSafeLookupTable(unsigned num_buckets = 19, Hash const& hasher_ = Hash()) :
		    m_buckets(num_buckets), m_hasher(hasher_)
	    {
		    for (unsigned i = 0; i < num_buckets; ++i)
		    {
			    m_buckets[i].reset(new BuckedType);
		    }
	    }

	    ThreadSafeLookupTable(ThreadSafeLookupTable const& other) = delete;
	    ThreadSafeLookupTable& operator=(ThreadSafeLookupTable const& other) = delete;

    	Value value_for(Key const& key,	Value const& default_value = Value()) 
	    {
		    return get_bucket(key).value_for(key, default_value);
	    }

	    void add_or_update_mapping(Key const& key, Value const& value)
	    {
		    get_bucket(key).add_or_update_mapping(key, value);
	    }

	    void remove_mapping(Key const& key)
	    {
		    get_bucket(key).remove_mapping(key);
	    }

	    std::map<Key, Value> get_map() 
	    {
		    std::vector<std::unique_lock<std::shared_mutex>> locks;
		    for (unsigned i = 0; i < m_buckets.size(); ++i)
	    	{
		    	locks.push_back(std::unique_lock<std::shared_mutex>(m_buckets[i]->mutex));
		    }
		    std::map<Key, Value> res;
		    for (unsigned i = 0; i < m_buckets.size(); ++i)
		    {
			    //需用typename告诉编译器bucket_type::bucket_iterator是一个类型，以后再实例化
			    //当然此处可简写成auto it = buckets[i]->data.begin();
			    typename BuckedType::bucket_iterator it = m_buckets[i]->data.begin();
			    for (;it != m_buckets[i]->data.end();++it)
			    {
				    res.insert(*it);
			    }
		    }
		    return res;
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

private:
    int _data;
};

void TestThreadSafeHash() {
    std::set<int> removeSet;
    ThreadSafeLookupTable<int, std::shared_ptr<MyClass>> table;

    std::thread t1([&]() {
        for(int i = 0; i < 10; i++)
        {
            auto class_ptr =  std::make_shared<MyClass>(i); 
            table.add_or_update_mapping(i, class_ptr);
        }
    });

    std::thread t2([&]() {
        for (int i = 0; i < 10; )
        {
            auto find_res = table.value_for(i, nullptr);
            if(find_res)
            {
                table.remove_mapping(i);
                removeSet.insert(i);
                i++;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    std::thread t3([&]() {
        for (int i = 10; i < 20; i++)
        {
            auto class_ptr = std::make_shared<MyClass>(i);
            table.add_or_update_mapping(i, class_ptr);
        }
        });


    t1.join();
    t2.join();
    t3.join();

    for(auto & i : removeSet)
    {
        std::cout << "remove data is " << i << std::endl;
    }

    auto copy_map =  table.get_map();
    for(auto & i : copy_map)
    {
        std::cout << "copy data is " << *(i.second) << std::endl;
    }
}

int main()
{
    /*
     *  Safe search map:
     *  1. the map based on red black tree is hard to impl with multi threads, as many node will change,
     *     we can first lock father, then children... But for now, we just impl unordered_map like.
     *  2. read is concurrent, and will block write. Write is sequntial, and will block read and write.
     *  3. list lock's grainularity is too coarse, not fine-grained lock. see safe_list.cpp for improve.
     * */
    TestThreadSafeHash();
}
