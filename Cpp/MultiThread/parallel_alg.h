#pragma once

#include <list>
#include <future>
#include <algorithm>

#include "thread_safe_container.h"
#include "join_thread.h"

/*

*/

template<typename Iterator, typename Func> // divide method 1 : divide data to threads
void parallel_for_each(Iterator first, Iterator last, Func f)
{
    unsigned long const length = std::distance(first, last);
    if (!length)
        return;
    unsigned long const min_per_thread = 25;
    unsigned long const max_threads =
        (length + min_per_thread - 1) / min_per_thread;
    unsigned long const hardware_threads =
        std::thread::hardware_concurrency();
    unsigned long const num_threads =
        std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    unsigned long const block_size = length / num_threads;

    std::vector<std::future<void>> futures(num_threads - 1);   //⇽-- - 1
    std::vector<std::thread> threads(num_threads - 1);
    JoinThreads joiner(threads);
    Iterator block_start = first;

    for (unsigned long i = 0; i < (num_threads - 1); ++i)
    {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        std::packaged_task<void(void)> task([=]()
        { // ⇽-- - 2
            std::for_each(block_start, block_end, f);
        });
        futures[i] = task.get_future();
        threads[i] = std::thread(std::move(task));    //⇽-- - 3
        block_start = block_end;
    }
    std::for_each(block_start, last, f);
    for (unsigned long i = 0; i < (num_threads - 1); ++i)
    {
        futures[i].get();   // ⇽-- - 4
    }
}

template<typename Iterator, typename Func> // divide method 2 : divide recursivly
void async_for_each(Iterator first, Iterator last, Func f)
{
    unsigned long const length = std::distance(first, last);
    if (!length) return;
    unsigned long const min_per_thread = 25;

    if (length < (2 * min_per_thread))
    {
        std::for_each(first, last, f);    //⇽-- - 1
    }
    else
    {
        Iterator const mid_point = first + length / 2;
        //⇽-- - 2
        std::future<void> first_half = std::async(&async_for_each<Iterator, Func>,
                first, mid_point, f);
        //⇽-- - 3
        async_for_each(mid_point, last, f); 
        // ⇽-- - 4
        first_half.get();   
    }
}

template<typename Iterator, typename MatchType> // diff from for_each : needs early return
Iterator parallel_find(Iterator first, Iterator last, MatchType match)
{
    struct find_element    //⇽-- - 1
    {
        void operator()(Iterator begin,Iterator end,
                        MatchType match,
                        std::promise<Iterator>* result,
                        std::atomic<bool>* done_flag)
        {
            try
            {
                for (; (begin != end) && !done_flag->load(); ++begin)    //⇽-- - 2
                {
                    if (*begin == match)
                    {
                        result->set_value(begin);    //⇽-- - 3
                        done_flag->store(true);    //⇽-- - 4
                        return;
                    }
                }
            }
            catch (...)    //⇽-- - 5
            {
                try
                {
                    result->set_exception(std::current_exception());    //⇽-- - 6
                    done_flag->store(true);
                }
                catch (...)    //⇽-- - 7
                {}
            }
        }
    };

    unsigned long const length = std::distance(first, last);
    if (!length)
        return last;
    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length + min_per_thread - 1) / min_per_thread;
    unsigned long const hardware_threads = std::thread::hardware_concurrency();
    unsigned long const num_threads = std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    unsigned long const block_size = length / num_threads;

    std::promise<Iterator> result;    //⇽-- - 8
    std::atomic<bool> done_flag(false);     //⇽-- - 9
    std::vector<std::thread> threads(num_threads - 1); //⇽-- - 10

    {    
        JoinThreads joiner(threads);
        Iterator block_start = first;
        for (unsigned long i = 0; i < (num_threads - 1); ++i)
        {
            Iterator block_end = block_start;
            std::advance(block_end, block_size);
            // ⇽-- - 11
            threads[i] = std::thread(find_element(), block_start, block_end, match, &result, &done_flag);
            block_start = block_end;
        }
        // ⇽-- - 12
        find_element()(block_start, last, match, &result, &done_flag);   
    }

    // ⇽-- - 13
    if (!done_flag.load())   
    {
        return last;
    }
    //⇽-- - 14
    return result.get_future().get();    
}

template<typename Iterator, typename MatchType>
Iterator parallel_find_impl(Iterator first, Iterator last, MatchType match,
    std::atomic<bool>& done)   // ⇽-- - 1
{
    try
    {
        unsigned long const length = std::distance(first,last);
        unsigned long const min_per_thread = 25;   // ⇽-- - 2
        if (length < (2 * min_per_thread))    //⇽-- - 3
        {
            for (; (first != last) && !done.load(); ++first)     //⇽-- - 4
            {
                if (*first == match)
                {
                    done = true;    //⇽-- - 5
                    return first;
                }
            }
            return last;    //⇽-- - 6
        }
        else
        {
            //⇽-- - 7
            Iterator const mid_point = first + (length / 2);   
            //⇽-- - 8
            std::future<Iterator> async_result = std::async(&parallel_find_impl<Iterator,MatchType>,    
                           mid_point,last,match,std::ref(done));
            //⇽-- - 9
            Iterator const direct_result = parallel_find_impl(first,mid_point,match,done); 
            //⇽-- - 10
            return (direct_result == mid_point) ?async_result.get() : direct_result;    
        }
    }
    catch (...)
    {
        // ⇽-- - 11
        done = true;   
        throw;
    }
}

template<typename Iterator, typename MatchType>
Iterator parallel_find_async(Iterator first, Iterator last, MatchType match)
{
    std::atomic<bool> done(false);
    //⇽-- - 12
    return parallel_find_impl(first, last, match, done);    
}

template<typename T>
struct Sorter  //1  
{
    struct SortChunk
    {
        std::list<T> data;
        std::promise<std::list<T>> promise;
    };

    ThreadSafeStackWaitable<SortChunk> chunks;    //⇽-- - 2
    std::vector<std::thread> threads;   // ⇽-- - 3
    unsigned const max_thread_count;
    std::atomic<bool> end_of_data;
    
    Sorter() : max_thread_count(std::thread::hardware_concurrency() - 1), end_of_data(false) {}
    
    ~Sorter()    //⇽-- - 4
    {
        end_of_data = true;     //⇽-- - 5
        for (unsigned i = 0; i < threads.size(); ++i)
        {
            threads[i].join();    //⇽-- - 6
        }
    }
    
    void try_sort_chunk()
    {
        std::shared_ptr<SortChunk> chunk = chunks.try_pop();    //⇽-- - 7
        if (chunk)
        {
            sort_chunk(chunk);    //⇽-- - 8
        }
    }
    
    std::list<T> do_sort(std::list<T>& chunk_data)    //⇽-- - 9
    {
        if (chunk_data.empty())
        {
            return chunk_data;
        }
        std::list<T> result;
        result.splice(result.begin(),chunk_data,chunk_data.begin());
        T const& partition_val = *result.begin();
        typename std::list<T>::iterator divide_point = std::partition(chunk_data.begin(),chunk_data.end(), // 10
                           [&](T const& val) { return val < partition_val; });
        SortChunk new_lower_chunk;
        new_lower_chunk.data.splice(new_lower_chunk.data.end(),
                                    chunk_data,chunk_data.begin(),
                                    divide_point);
        std::future<std::list<T> > new_lower =
            new_lower_chunk.promise.get_future();
        chunks.push(std::move(new_lower_chunk));   // ⇽-- - 11
        if (threads.size() < max_thread_count)    // ⇽-- - 12
        {
            threads.push_back(std::thread(&Sorter<T>::sort_thread, this));
        }
        std::list<T> new_higher(do_sort(chunk_data));
        result.splice(result.end(),new_higher);
        while (new_lower.wait_for(std::chrono::seconds(0)) !=
              std::future_status::ready)    //⇽-- - 13
        {
            try_sort_chunk();   // ⇽-- - 14
        }
        result.splice(result.begin(),new_lower.get());
        return result;
    }
    
    void sort_chunk(std::shared_ptr<SortChunk> const& chunk)
    {
        chunk->promise.set_value(do_sort(chunk->data));    //⇽-- - 15
    }
    
    void sort_thread()
    {
        while (!end_of_data)    //⇽-- - 16
        {
            try_sort_chunk();    // ⇽-- - 17
            //交出时间片, other thread can push data to chunks
            std::this_thread::yield();    //⇽-- - 18
        }
    }
};

template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input)    //⇽-- - 19
{
    if (input.empty())
    {
        return input;
    }
    Sorter<T> s;
    return s.do_sort(input);    //⇽-- - 20
}

template<typename Iterator>
void parallel_partial_sum(Iterator first, Iterator last)
{
    typedef typename Iterator::value_type value_type;

    struct process_chunk    //⇽-- - 1
    {
        void operator()(Iterator begin, Iterator last,
            std::future<value_type>* previous_end_value,
            std::promise<value_type>* end_value)
        {
            try
            {
                Iterator end = last;
                ++end;
                std::partial_sum(begin, end, begin);    //⇽-- - 2
                if (previous_end_value)    //⇽-- - 3
                {
                    value_type addend = previous_end_value->get();   // ⇽-- - 4
                    *last += addend;   // ⇽-- - 5
                    if (end_value)
                    {
                        end_value->set_value(*last);    //⇽-- - 6
                    }
                    // ⇽-- - 7
                    std::for_each(begin, last, [addend](value_type& item)
                        {
                            item += addend;
                        });
                }
                else if (end_value)
                {
                    // ⇽-- - 8
                    end_value->set_value(*last);
                }
            }
            catch (...)  // ⇽-- - 9
            {
                if (end_value)
                {
                    end_value->set_exception(std::current_exception());   // ⇽-- - 10
                }
                else
                {
                    throw;   // ⇽-- - 11
                }

            }
        }
    };
    unsigned long const length = std::distance(first, last);

    if (!length) {
        return;
    }
    unsigned long const min_per_thread = 25;     //⇽-- - 12
    unsigned long const max_threads = (length + min_per_thread - 1) / min_per_thread;
    unsigned long const hardware_threads = std::thread::hardware_concurrency();
    unsigned long const num_threads = std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    unsigned long const block_size = length / num_threads;
    typedef typename Iterator::value_type value_type;
    
    std::vector<std::thread> threads(num_threads - 1);   // ⇽-- - 13

    std::vector<std::promise<value_type>> end_values(num_threads - 1);   // ⇽-- - 14
    
    std::vector<std::future<value_type>> previous_end_values;   // ⇽-- - 15
    previous_end_values.reserve(num_threads - 1);   // ⇽-- - 16
    JoinThreads joiner(threads);
    Iterator block_start = first;
    for (unsigned long i = 0; i < (num_threads - 1); ++i)
    {
        Iterator block_last = block_start;
        std::advance(block_last, block_size - 1);   // ⇽-- - 17
        // ⇽-- - 18
        threads[i] = std::thread(process_chunk(), block_start, block_last,
            (i != 0) ? &previous_end_values[i - 1] : 0,
            &end_values[i]);
        block_start = block_last;
        ++block_start;   // ⇽-- - 19
        previous_end_values.push_back(end_values[i].get_future());   // ⇽-- - 20
    }
    Iterator final_element = block_start;
    std::advance(final_element, std::distance(block_start, last) - 1);   // ⇽-- - 21
    // ⇽-- - 22
    process_chunk()(block_start, final_element, (num_threads > 1) ? &previous_end_values.back() : 0, 0);
}