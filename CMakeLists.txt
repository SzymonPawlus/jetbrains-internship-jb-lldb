cmake_minimum_required(VERSION 3.16)
project(GlobalVariableWatch)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(gwatch gwatch.cpp elf.cpp process.cpp)
add_library(gwatch_lib STATIC elf.cpp process.cpp)

add_executable(gwatch_test test.cpp)
target_compile_options(gwatch_test PRIVATE -O0)

add_subdirectory(dependencies/googletest)
enable_testing()
add_subdirectory(tests)
