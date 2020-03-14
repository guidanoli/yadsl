add_library(graph graph.c graph.h)
target_link_libraries(graph set aa memdb)
target_include_directories(graph PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(graph PROPERTIES
	FOLDER libraries
	POSITION_INDEPENDENT_CODE ON)

if(AA_BUILD_TESTS)
	add_tester_module(graphtest SOURCES graph.test.c LINKS graph graphio graphsearch var)
	add_tester_scripts(graphtest SOURCES graph.script)
endif()

if(AA_PYTHON_SUPPORT)
	add_subdirectory(python)
endif()