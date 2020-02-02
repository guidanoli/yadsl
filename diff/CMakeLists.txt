add_library(diff diff.c diff.h)
add_tester_module(difftest SOURCES diff.test.c)
add_tester_scripts(difftest SOURCES diff.script)
target_link_libraries(difftest diff)