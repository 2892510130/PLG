#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <numeric>
#include <chrono>

void use_vector();
void param_function(int _param);

template<typename Iterator, typename T>
struct accumulate_block
{
	void operator()(Iterator first, Iterator last, T& result)
	{
		result = std::accumulate(first, last, result);
	}
};

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init);
void use_parallel_acc();
void non_parallel_acc();

int main()
{
    /*
        Thread Control:
        1. can not move a thread to a thread var already binded
        2. joining thread : join when deconstruct (std::thread will not, it will terminate)
        3. in container use emplace back not push back (it will call copy constructor, but thread do not have one)
        4. std::thread::hardware_concurrency() get the system core number 
           4.1 you do not want create more threads than this, otherwise it will take many time for the switch
        5. get_id() to get an id
    */

    use_vector();

    use_parallel_acc();

    non_parallel_acc();

    std::thread t([](){
        std::cout << "in thread id " << 
        std::this_thread::get_id() << std::endl;
        std::cout << "thread start" << std::endl;
    });

    t.join();
}

void param_function(int _param)
{
    _param += 1;
}

void use_vector() 
{
    std::vector<std::thread> threads;
    for (unsigned i = 0; i < 2; ++i) {
        threads.emplace_back(param_function, i);
    }

    for (auto& entry : threads) {
        std::cout << entry.get_id() << '\n';
        entry.join();
    }
}

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init)
{
    unsigned long const length = std::distance(first, last);
    if (!length)
        return init;
    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length + min_per_thread - 1) / min_per_thread;
    unsigned long const hardware_threads = std::thread::hardware_concurrency();
    unsigned long const num_threads = std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    unsigned long const block_size = length / num_threads;
    std::cout << "hardware threads: " << hardware_threads << '\n';

    std::vector<T> results(num_threads);
    std::vector<std::thread> threads(num_threads - 1);
    Iterator block_start = first;
    for (unsigned long i = 0; i < (num_threads - 1); ++i)
    {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        threads[i] = std::thread(accumulate_block<Iterator, T>(), block_start, block_end, std::ref(results[i]));
        block_start = block_end;
    }
    accumulate_block<Iterator, T>()(block_start, last, results[num_threads - 1]);

    for (auto& entry : threads) entry.join();
    return std::accumulate(results.begin(), results.end(), init);
}

void use_parallel_acc() 
{
    std::vector <int> vec(10000);
    for (int i = 0; i < 10000; i++) {
        vec.push_back(i);
    }
    int sum = 0;

    auto start = std::chrono::high_resolution_clock::now();
    sum = parallel_accumulate<std::vector<int>::iterator, int>(vec.begin(), vec.end(), sum);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Parallel takes " << duration << "ms\n";

    std::cout << "parallel sum is " << sum << std::endl;
}

void non_parallel_acc()
{
    std::vector <int> vec(10000);
    for (int i = 0; i < 10000; i++) {
        vec.push_back(i);
    }

    auto start = std::chrono::high_resolution_clock::now();
    int sum = 0;
    sum = std::accumulate(vec.begin(), vec.end(), sum);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "No parallel takes " << duration << "ms\n";
    std::cout << "no parallel sum is " << sum << std::endl;
}