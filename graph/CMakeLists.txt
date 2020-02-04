add_library(graph graph.c graph.h)
target_link_libraries(graph set)
target_include_directories(graph PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_tester_module(graphtest SOURCES graph.test.c LINKS graph graphio graphsearch var)
add_tester_scripts(graphtest SOURCES graph.script)