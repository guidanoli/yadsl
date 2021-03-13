add_yadsl_library(yatester STATIC
    SOURCES
    yatester.c parser.c runner.c cmdhdl.c
    cmd.h err.h parser.h runner.h yatester.h cmdhdl.h)

target_link_libraries(yatester argvp)

add_executable(yatestertest yatester.test.c)
target_link_libraries(yatestertest yatester)
set_target_properties(yatestertest PROPERTIES FOLDER tests)
