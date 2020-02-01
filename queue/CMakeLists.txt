add_library(queue queue.c queue.h)
add_tester_module(queuetest SOURCES queue.test.c LINKS queue var)
add_tester_script(queuetest SOURCES queue.script)
target_include_directories(queue PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})