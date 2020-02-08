add_library(graphio SHARED graphio.c graphio.h)
target_link_libraries(graphio graph map)
target_include_directories(graphio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(AA_BUILD_TESTS)
	add_tester_scripts(graphtest SOURCES graphio.script)
endif()