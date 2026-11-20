cmake_minimum_required(VERSION 3.30)

project(TinyComiler)

# Basic settings
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(compiler_main 
    src/main.cpp
    src/lexer.cpp
    src/ast.cpp
    src/ast_dump.cpp
    src/parser.cpp
    src/checker.cpp
    src/interp.cpp
)
target_include_directories(compiler_main PRIVATE ./include)