cmake_minimum_required(VERSION 3.16.1)
project(cpp-examples)

# Project setup
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
#file(COPY etc  DESTINATION ${CMAKE_BINARY_DIR})

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
endif(CCACHE_FOUND)


find_package(SQLite3 REQUIRED)
add_executable(sqlite3_simple sqlite3_simple.cpp)
target_include_directories(sqlite3_simple PUBLIC /usr/include)
target_link_libraries(sqlite3_simple PUBLIC sqlite3)

# find_package(nanodbc REQUIRED)

# add_executable(nanoodbc_test nanoodbc_test.cpp)
# target_include_directories(nanoodbc_test PUBLIC /usr/local/include)
# target_link_libraries(nanoodbc_test PUBLIC nanodbc)

add_executable(span span.cpp)

# add_subdirectory(boost_socket)