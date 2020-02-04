add_library(set set.c set.h)
target_include_directories(set PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_tester_module(settest SOURCES set.test.c LINKS set var)
add_tester_scripts(settest SOURCES set.script)