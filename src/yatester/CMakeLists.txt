add_yadsl_library(yatester STATIC
    SOURCES
    config.c config.h
    errhdl.c errhdl.h
    parser.c parser.h
    runner.c runner.h
    main.c yatester.h)

target_link_libraries(yatester argvp)
