#include <vector>

#include "interrupt_thread.h"

std::vector<interruptible_thread> background_threads;
std::mutex mtx1;
std::mutex mtx2;
std::condition_variable cv1;
std::condition_variable_any cv2;

thread_local interrupt_flag this_thread_interrupt_flag;
thread_local int cnt = 0;

void interruptible_wait(std::condition_variable& cv,
    std::unique_lock<std::mutex>& lk)
{
    interruption_point();
    this_thread_interrupt_flag.set_condition_variable(cv);
    clear_cv_on_destruct guard;
    interruption_point();
    cv.wait_for(lk, std::chrono::milliseconds(1)); // use wait then we may have dead lock (notify before the wait, notify does not lock this lk)
    interruption_point();
}

void interruption_point()
{
   if (this_thread_interrupt_flag.is_set())
   {
        std::cout << std::this_thread::get_id() << ": " << ++cnt << '\n';
        throw thread_interrupted();
   }
}

void start_background_processing() {
    background_threads.push_back([]() {
        try {
            std::unique_lock<std::mutex> lock(mtx1);
            interruptible_wait(cv1, lock);
        }
        catch (std::exception& ex) {
            std::cout << "catch exception is " << ex.what() << std::endl;
        }
    
    });

    background_threads.push_back([]() {
        try {
            std::unique_lock<std::mutex> lock(mtx2);
            this_thread_interrupt_flag.wait(cv2, mtx2);
        }
        catch (std::exception& ex) {
            std::cout << "catch exception is " << ex.what() << std::endl;
        }

    });
}

int main()
{
    start_background_processing();
    for (unsigned i = 0; i < background_threads.size(); i++) {
        background_threads[i].interrupt();
    }

    for (unsigned i = 0; i < background_threads.size(); i++) {
        background_threads[i].join();
    }
}