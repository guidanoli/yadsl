add_aa_library(graphsearch
    SOURCES graphsearch.c graphsearch.h)
target_link_libraries(graphsearch graph queue)

if(AA_BUILD_TESTS)
	add_tester_scripts(graphtest SOURCES graphsearch.script)
endif()