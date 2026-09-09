cmake_minimum_required(VERSION 3.30)

project(LearnMultiThread) #  cmake -G "MinGW Makefiles" .. | cmake --build .

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(thread thread.cpp)
add_executable(thread_control thread_control.cpp)
add_executable(mutex_lock mutex_lock.cpp)
add_executable(safe_singleton safe_singleton.cpp)
add_executable(safe_queue safe_queue.cpp)
add_executable(future_promise_async future_promise_async.cpp)