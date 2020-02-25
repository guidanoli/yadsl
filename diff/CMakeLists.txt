add_library(diff diff.c diff.h)
target_link_libraries(diff ${LIBM_LIBRARIES} memdb)
target_include_directories(diff PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(diff PROPERTIES
	FOLDER libraries
	POSITION_INDEPENDENT_CODE ON)

if(AA_BUILD_TESTS)
	add_tester_module(difftest SOURCES diff.test.c LINKS diff)
	add_tester_scripts(difftest SOURCES diff.script)
endif()

if(AA_PYTHON_SUPPORT)
	add_subdirectory(python)
endif()