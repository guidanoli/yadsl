add_library(var var.c var.h)
add_tester_module(vartest SOURCES var.test.c LINKS var)
add_tester_script(vartest SOURCES var.script)
target_include_directories(var PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})