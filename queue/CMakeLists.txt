add_library(queue queue.c queue.h)
target_include_directories(queue PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(queue PROPERTIES FOLDER libraries)

if(AA_BUILD_TESTS)
	add_tester_module(queuetest SOURCES queue.test.c LINKS queue var)
	add_tester_scripts(queuetest SOURCES queue.script)
endif()