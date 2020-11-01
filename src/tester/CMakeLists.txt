add_yadsl_library(tester STATIC
    SOURCES tester.c tester.h)
target_link_libraries(tester argvp)

# Makes tester tools visible to global scope and allows tools to
# be included by other projects too.
if(YADSL_BUILD_TESTS)
	add_subdirectory(memfail)

	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
	include(AddTesterModule)
	include(AddTesterScripts)
	
	add_tester_module(testertest SOURCES tester.test.c)
    target_link_libraries(testertest testerutils)
	add_tester_scripts(testertest SOURCES tester.script)
endif()