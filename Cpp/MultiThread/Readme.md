Folder and File structure:
1. thread
2. thread_control
3. mutex_lock
4. safe_singleton
5. safe_queue
6. future_promise_async
7. thread_pool

Important:
- Use {} to constraint the scope of the lock and mutex
- when use cond var first lock (and before wait we must use unique_lock because wait will unlock)
- when use detach we must make sure the main thread will not end too early