#include "thread_safe_container.h"

void test_safe_queue()
{
    ThreadSafeQueuePtr<std::vector<int>> tsqp;
    std::vector<int> data{1, 2, 3, 4, 5};
    tsqp.push(data);
    tsqp.test();
    std::shared_ptr<std::vector<int>> res = tsqp.try_pop();
    std::cout << "data: " << (*res)[1] << '\n';
    std::cout << "ptr: " << res << '\n';
}

int main()
{
    /*
     *  Thread safe stack and queue:
     *  1. in mutex_loc.cpp we implement a thread safe queue with mutex and throw exception,
     *     we now upgrade it.
     *     1.1 we should not throw exception when stack is empty (as it's a common situation). 
     *     So we use condition_variable and impl two pop method, one wait, one doesn't.
     *     1.2 only one problem : first for vector like data, we may get exception when we move the data (memory
     *     not enough, even though we use move, some condition like user defined allocator, we
     *     may re-allocate), now this thread do not notify_one, ohter thread can not wake, 
     *     and stack top is moved. We can fix it by using stack<shared_ptr<T>>, see the safe queue.
     *  2. in safe_queue.cpp we impl a thread safe queue, new we upgrade it with save shared_ptr.
     *     when pop we return a pointer, so the memory will be enough (most most of the time);
     *     2.1 However the push and pop are sequential, we need to seperate them. We can use a 
     *     cycle queue like in no_lock_queue, but here we impl another way (using a virtual point).
     *     tail always point to this virtual node, and head node move forward and point to the next;
     * */
    test_safe_queue();
}

