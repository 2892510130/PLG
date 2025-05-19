#include "lockfree_stack.h"

int main()
{
    /*
     *  Lock Free Stack
     *  1. if we delete the data right after 4, we may have some problem. Two threads, 
     *     one finish delete, old_head point to A, real head step to B. Second thread 
     *     in first while, old_head still point to A, but A is deleted, it is nullptr,
     *     when do compare_exchange_weak we give param of old_head->next, but old_head
     *     is freed, we get a use-after-free. Error!
     *  2. How to fix it? Use shared_ptr to save head, rather than origin * pointer. Or
     *     use so-called Hazard pointer, delete the data later. Just see try_reclaim.
     *  3. one problem left: if we always encounter threads_in_pop > 1, to_be_deleted will
     *     never be reclaim. We can let the last thread do pop reclaim it, see hazard_pointer.cpp.
     * */
}
