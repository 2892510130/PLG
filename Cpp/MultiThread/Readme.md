ref: https://www.yuque.com/lianlianfengchen-cvvh2.

Folder and File structure:
1.  thread
2.  thread_control
3.  mutex_lock
4.  safe_singleton
5.  safe_queue
6.  future_promise_async
7.  thread_pool
8.  quick_sort_mutli_thread
9.  actor_csp
10. atomic_memory_model (and no_lock_queue with circular queue)
11. fence
12. thread_safe_containter
13. safe_map
14. safe_list
15. no_lock_stack
16. hazard_pointer
17. ref_count_lockfree_stack
18. lockfree_queue (it's not circular queue)

Important:
- Use {} to constraint the scope of the lock and mutex
- when use cond var first lock (and before wait we must use unique_lock because wait will unlock)
- when use detach we must make sure the main thread will not end too early
- compare_exchange will do bit-wise compare!
- fetch_add will return value before add
- release -> write in same thread / read-modify-write in any order in any thread -> acquire, release sync with acquire
    - for any release model of atomic M, any read-modify-write with any model can read the latest M.

Principle:
1. first use seq_cst order, then optimize.
2. always keep memory if some thread maybe working on it.
    1. free memory when data structur is freed
    2. Hazard pointers store the used pointer
    3. ref count
    4. GC, or circular data structure.
3. A-B-A problem, in compare-exchange alg: t1 read x = A, t2 read x = A then change it to B, t3 change A (for 
example A is a pointer, freed it or change it), t4 change x back to A. Now t1 compare x with old_x, both equal
to A, but what A points to or hold is changed!
    1. How to fix this problem, create a ABA counter, when change x, not only change the data, but also the counter.
    2. the lockfree data in this code do not have ABA problem because everytime we pop and push different data, as our
    data is a T data and a pointer, the pointer will be different even if T is the same.
4. let the busy waiting thread help other thread, see lockfree_queue.cpp.
