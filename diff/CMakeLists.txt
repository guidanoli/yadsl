add_library(diff SHARED diff.c diff.h)
target_include_directories(diff PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(AA_BUILD_TESTS)
	add_tester_module(difftest SOURCES diff.test.c LINKS diff)
	add_tester_scripts(difftest SOURCES diff.script)
endif()

if(AA_PYTHON_SUPPORT)
	add_subdirectory(python)
endif()