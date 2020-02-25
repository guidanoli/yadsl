add_library(set set.c set.h)
target_link_libraries(set memdb)
target_include_directories(set PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(set PROPERTIES
	FOLDER libraries
	POSITION_INDEPENDENT_CODE ON)

if(AA_BUILD_TESTS)
	add_tester_module(settest SOURCES set.test.c LINKS set var)
	add_tester_scripts(settest SOURCES set.script)
endif()