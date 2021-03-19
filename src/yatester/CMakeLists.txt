add_yadsl_library(yatester STATIC
    SOURCES
    yatester.c parser.c runner.c cmdhdl.c builtins.c
    cmd.h err.h parser.h runner.h yatester.h cmdhdl.h builtins.h status.h)

target_link_libraries(yatester argvp ${LIBM_LIBRARIES})

if(YADSL_BUILD_TESTS)
    add_subdirectory(tests)
endif()
