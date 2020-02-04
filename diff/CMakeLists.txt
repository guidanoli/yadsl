add_library(diff diff.c diff.h)

add_tester_module(difftest SOURCES diff.test.c LINKS diff)
add_tester_scripts(difftest SOURCES diff.script)