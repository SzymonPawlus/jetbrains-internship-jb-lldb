cmake_minimum_required(VERSION 3.16)
project(GlobalVariableWatch)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(gwatch gwatch.cpp elf.cpp)
add_executable(gwatch_test test.cpp)
