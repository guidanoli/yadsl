add_library(var var.c var.h)
target_include_directories(var PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_tester_module(vartest SOURCES var.test.c LINKS var)
add_tester_scripts(vartest SOURCES var.script)