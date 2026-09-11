## thread
- `std::thread` use `join` to wait for it to complete, `std::jthread` will auto join when release.
  - implement a `thread_guard` class to release it in deconstructor, `std::jthread` does not need it!
- We can use ***factor*** and ***lambda*** as the task.
- `detach` to run in background, `std::thread` does not hold any handle, so make sure it will not access some local variables.
  - use `join` most of the time, use `detach` if you are confident enough.
- The parameter send to thread is by value, so be careful with the implicit convert.
  - so when you want to send a parameter as reference, use `std::ref`.
- If you want to bind a class function to a thread, use `&`, and pass `&instance`.
- We can not copy, but we can move.

## thread control
- We can not move a thread to a thread already binded to task.
- In container use emplace back instead of push back, as thread does not have copy constructor.
- `std::thread::hardware_concurrency` get the system core number.
  - You do not want create more threads than this, otherwise it will take many time for the switch.
- `get_id` to get the thread id.

## mutex lock
- Do `lock` and `unlock`, or use a ***lock guard***.
- Thread safe data, when pop check empty and return nullptr or throw.
- ***death lock***
  - example: thread 1 first lock A then do something then lock B, thread 2 first lock B then lock A
  - what to do: only lock and unlock one mutex
  - but sometime we need two lock in one function:
    - lock at the same time: lock A and lock B then do something, `std::lock(mtx1, mtx2)` and use guard to adopt these 2 mutex.
    - or use `std::scoped_lock guard(mtx1, mtx2)`
    - use hierarchical mutex, avoid loop lock, first lock higher weight lock
- ***unique lock***
  - same as lock_guard but we can manually unlock it.
  - we can defer it by `std::unique_lock<std::mutex> lock(mtx1, std::defer_lock)`
  - we can adopt a lock by `std::unique_lock<std::mutex> lock(mtx1, std::adopt_lock)`, and use `owns_lock` to check ownership
  - mutex can not be moved but unique_lock can
- ***shared lock***
  - if we want multiple threads read same data in the same time, we can use `shared_mutex` and `shared_lock`.
    - we can not write data, but if we use `lock_guard` we can (but now we can not read by multiple threads).
  - `shared_timed_mutex` can send in a time out, if in this timeout it can not get lock return false

## safe singleton
- ***Magic Static***: after c++ 11, we can use local static var, it is thread safe. 
  - it is one of the lazy mode. We will initialize it when `get_inst` is called.
- After c++ 17, we can direct use `inline` to define the instance in the class
  - hunger mode: initialize the singleton from main thread once (before the `main` function)
- Other notes in the cpp file is the lagacy problem, it is solved by magic static.
  - afther c++ 11, magic static is the only solution for singleton
- If we want to create a singleton base class that the derived class can inherit it to get the singleton feature
  - use **CRTP**: Curiously Recurring Template Pattern, as shown in the cpp file.
  - for more strong version use `std::atomic` and mutex.

## safe queue
- `std::condition_variable` cv
  - `cv.wait(lock, cond)` if cond is true then continue to run the program
  - if cond is false, then unlock (let other thread do their work), this thread get into the wait loop.
- `cv.notify_one` will wake up a thread in the wait loop, then that thread will try to get mutex, after it finnally get the mutex (unlocked) then do the wait again.

## future promise async
- `std::async` will open a thread and bind to a `std::promise` object, and can be get by a `std::future`
- use `get` to get the result, it will block the thread until it get the result, it can only use once
  - deconstructor will also block the thread (call wait), see block_async.
  - use try catch to get the future result.
  - future will deconstruct when the last object holds it deconstruct, as we create a shared pointer of the future in async
- `wait`: same as get but will not return result and it can use any times
- `wait_for` and `wait_until` to check whether the async is done for a timestep
- `std::packaged_task` can pack a task on a thread and bind a future to get result
- `std::promise` can send a value or exception on a thread, can bind a future to get result
  - they are a pair, so when you free a promise you can not use the future (it will crash)
- `std::shared_future` for multiple threads wait for a async task's result, can call `get` multiple times

## thread pool
- Easy implementation in thread.h, with magic static.
- More complicated can see steal thread pool, like https://github.com/Lallapallooza/citor.

## actor and csp
- They are two design pattern
  - do not communicate with shared memory, share memory with communicating!!
- Actor does not need shared value, it only process one message one time, so it is safe.
  - It knows who will receive and who give the message (has address). It may not need lock.
- CSP (communicating sequential process): focus on the channel, know nothing about recevier and giver.

## atomic memory model
- For complicated scene, first consider mutex, if you have to use atomic, use `seq_cst`, then after you are sure, down to `acquire/release` or `relaxed`.
- `std::atomic_flag` is the base class for other atomic class (only some basic type and pointer, and after c++ 20 smart pointer), they can make sure the atomic feature.
  - `store`, `load`, `exchange` | RMW (read modify write) -> `fetch_*` like `fetch_add` | `compare_exchange_strong` and `compare_exchange_weak` (this often put in a while loop as it allows failure)
  - ```cpp
        void lock()
        {
            while (flag.test_and_set(std::memory_order_acquire)); // a spin lock
        }

        void unlock()
        {
            flag.clear(std::memory_order_release);
        }
    ```
- We can set mode on the memory type
- | Memory Order | Semantics | Typical Use |
  |--------------|-----------|-------------|
  | `memory_order_relaxed` | Atomicity only, no ordering guarantee | Counters, statistics |
  | `memory_order_consume` | Dependency ordering (rarely used, not recommended) | Almost never used |
  | `memory_order_acquire` | Read op: later reads/writes cannot be reordered before it | Locking, reading shared data |
  | `memory_order_release` | Write op: earlier reads/writes cannot be reordered after it | Unlocking, publishing data |
  | `memory_order_acq_rel` | Both acquire + release | CAS, RMW |
  | `memory_order_seq_cst` | Global sequential consistency (default, strongest) | Simple and correct, slightly slower |
- release + acquire -> ***synchronizes-with*** | cross thread visibility guarantees
  - ***sequenced-before***, in thread order
  - ***happens-before*** -> transitivity + synchronizes-with + sequenced-before
  - A synchronizes-with B  ⟹  A happens-before B, but not reversed
- Hardware Background: Multi-Core Cache Hierarchy
    - Multiple CPU cores, each core has its own store buffer / L1 cache
    - Two cores share an L2 cache
    - All cores share L3 cache and main memory
- How to ensure data consistency?
    - MESI protocol, a Write-Invalidate protocol
    - Only 1 CPU can write, others read
    - After writing to cache, broadcast an "Invalidate" to all other CPU cores
    - MESI has 4 states:
        - M (Modified): modified locally, dirty, other CPUs' copies invalid
        - E (Exclusive): only in this CPU's cache, clean, free to read/write
        - S (Shared): present in multiple caches, clean, all can read
            - To write, must first broadcast, invalidate all other copies
            - Only 1 CPU can write
            - However, all CPUs can read, so they may read old data (broadcast not yet arrived)
            - So we need Memory Order
        - I (Invalid): invalid, must fetch from memory or another cache
- Summary
    - MESI solves hardware-level cache coherence
    - Memory Order solves software-level ordering and visibility

## fence (memory barrier)
- Forbid the cpu random re-order
- ```cpp
    x.store(true, std::memory_order_relaxed);  //1
    std::atomic_thread_fence(std::memory_order_release);  //2, make sure 1 before 2
    y.store(true, std::memory_order_relaxed);  //3
    ```
- ```cpp
    while (!y.load(std::memory_order_relaxed));  //4
    std::atomic_thread_fence(std::memory_order_acquire); //5, make sure 6 read after 5
    if (x.load(std::memory_order_relaxed))  //6
        ++z;
    ```

## lock free container and thread safe container
- We can implement a circular queue (tail and head design) with lock, with atomic spin
  - or we can CAS design to make pop and push do not block each other
- We can use one lock and condition variable to make thread safe stack and queue
- We can use two lock to implement thread safe queue (with linked list)
- `template<typename Key, typename Value, typename Hash = std::hash<Key>>` as `class ThreadSafeLookupTable`
  - we need the inner data type
  - ```cpp
        class BuckedType
        {
        private:
            friend class ThreadSafeLookupTable;

            typedef std::pair<Key, Value> bucket_value;
            typedef std::list<bucket_value> bucket_data;
            typedef typename bucket_data::iterator bucket_iterator;

            bucket_data data;
            mutable std::shared_mutex mutex;
        }
    ```
- Safe list, no lock stack, no lock queue, lock free queue, lock free stack

## hazard pointer
- In no lock stack, we mentioned if always we have multiple threads pop, the node should be reclaimed may never reclaim, it's length will keep growing.
  - so our approach is to let the last thread that pop do the reclaim, it's like GC. Each therad has it's own hazard pointer, other threads can not delete it.
  - but we know GC like is slow! We have to walk through the hazard_pointer for outstanding_hazard_pointers_for. We can use some space trade for time tech, like create 2N hazard_pointer, only for n > N we do this walk.

## ref count lock free stack
- We know it is hard to safly delete a node in lock free stack
- So we can use external (how many thread are holding/accessing this node) ref counter and internal ref counter to solve it
- It is complicated, ref counter is expensive