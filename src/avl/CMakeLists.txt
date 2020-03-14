add_library(avl avl.c avl.h)
target_include_directories(avl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(avl memdb)
set_target_properties(avl PROPERTIES
	FOLDER libraries
	POSITION_INDEPENDENT_CODE ON)

if(AA_BUILD_TESTS)
	add_tester_module(avltest SOURCES avl.test.c LINKS avl aa)
	add_tester_scripts(avltest SOURCES avl.script)
endif()

if(AA_PYTHON_SUPPORT)
	add_subdirectory(python)
endif()
