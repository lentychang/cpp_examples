cmake_minimum_required(VERSION 3.16.1)
project(nanodbc-test)


find_package(SQLite3 REQUIRED)
add_executable(sqlite3_simple sqlite3_simple.cpp)
target_include_directories(sqlite3_simple PUBLIC /usr/include)
target_link_libraries(sqlite3_simple PUBLIC sqlite3)

find_package(nanodbc REQUIRED)

add_executable(nanoodbc_test nanoodbc_test.cpp)
target_include_directories(nanoodbc_test PUBLIC /usr/local/include)
target_link_libraries(nanoodbc_test PUBLIC nanodbc)