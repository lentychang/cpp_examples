add_executable(template_with_const_char_arg template_with_const_char_arg.cpp)

add_library(integer integer.cpp)
target_include_directories(integer PUBLIC {CMAKE_CURRENT_SOURCE_DIR})

add_executable(integer_main integer_main.cpp)
target_include_directories(integer_main PUBLIC {CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(integer_main PUBLIC integer)
#endif // __CMAKELISTS_H__