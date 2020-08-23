add_yadsl_library(graphio
    SOURCES graphio.c graphio.h)
target_link_libraries(graphio graph map)

if(YADSL_BUILD_TESTS)
	add_tester_scripts(graphtest SOURCES graphio.script)
endif()