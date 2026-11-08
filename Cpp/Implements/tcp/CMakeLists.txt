cmake_minimum_required(VERSION 3.26)

project(TcpLearning)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(tcp_main
    main.cpp
    tcp_basic.cpp
)