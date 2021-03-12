add_yadsl_library(yatester STATIC
    SOURCES yatester.c errhdl.c parser.c runner.c cmdhdl.c
    cmd.h err.h errhdl.h parser.h runner.h yatester.h cmdhdl.h)

target_link_libraries(yatester argvp)
