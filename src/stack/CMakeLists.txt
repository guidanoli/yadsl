add_library(stack stack.c stack.h)
target_link_libraries(stack memdb)
target_include_directories(stack PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(stack PROPERTIES
	FOLDER libraries
	POSITION_INDEPENDENT_CODE ON)

if(AA_BUILD_TESTS)
	add_tester_module(stacktest SOURCES stack.test.c LINKS stack)
	add_tester_scripts(stacktest SOURCES stack.script)
endif()

if(AA_PYTHON_SUPPORT)
	add_subdirectory(python)
endif()
