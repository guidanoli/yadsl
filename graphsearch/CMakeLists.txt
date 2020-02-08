add_library(graphsearch SHARED graphsearch.c graphsearch.h)
target_link_libraries(graphsearch graph queue)
target_include_directories(graphsearch PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(AA_BUILD_TESTS)
	add_tester_scripts(graphtest SOURCES graphsearch.script)
endif()