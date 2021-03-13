add_yadsl_library(yatester STATIC
    SOURCES
    yatester.c parser.c runner.c cmdhdl.c builtins.c
    cmd.h err.h parser.h runner.h yatester.h cmdhdl.h builtins.h)

target_link_libraries(yatester argvp)

add_subdirectory(tests)
