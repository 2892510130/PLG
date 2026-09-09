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