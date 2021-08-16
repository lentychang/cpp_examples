find_package(SQLite3 REQUIRED)
find_library(MYSQLCONN_LIB mysqlcppconn /usr/lib /usr/lib/x86_64-linux-gnu REQUIRED)
set(MYSQLCONN_INC /usr/include /usr/include/cppconn/)

add_executable(db_static db_static.cpp)
target_include_directories(db_static PUBLIC ${MYSQLCONN_INC} ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(db_static PUBLIC ${MYSQLCONN_LIB} sqlite3)


add_library(db_static_sqlite db_static_sqlite.cpp)
target_include_directories(db_static_sqlite PUBLIC ${MYSQLCONN_INC} ${CMAKE_CURRENT_SOURCE_DIR}}/include)
target_link_libraries(db_static_sqlite PUBLIC ${MYSQLCONN_LIB} sqlite3)

add_executable(static_sqlite_main static_sqlite_main.cpp)
target_link_libraries(static_sqlite_main PRIVATE db_static_sqlite)

add_executable(db_dynamic db_dynamic.cpp)
target_include_directories(db_dynamic PUBLIC ${MYSQLCONN_INC} ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(db_dynamic PUBLIC ${MYSQLCONN_LIB} sqlite3)



