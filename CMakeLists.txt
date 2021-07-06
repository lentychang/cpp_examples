cmake_minimum_required(VERSION 3.16.1)
project(nanodbc-test)

find_package(nanodbc REQUIRED)

add_executable(nanoodbc_test nanoodbc_test.cpp)
target_include_directories(nanoodbc_test PUBLIC /usr/local/include)
target_link_libraries(nanoodbc_test PUBLIC nanodbc)