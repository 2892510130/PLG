cmake_minimum_required(VERSION 3.26)

project(TcpLearning)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_library(tcp_basic STATIC tcp_basic.cpp tcp_client.cpp tcp_server.cpp)
target_include_directories(tcp_basic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tcp_basic PUBLIC Threads::Threads)

add_executable(tcp_test test_server_client.cpp)
target_link_libraries(tcp_test PRIVATE tcp_basic)

add_executable(tcp_main main.cpp)