add_library(graphio graphio.c graphio.h)
target_link_libraries(graphio graph map common)
target_include_directories(graphio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(graphio PROPERTIES FOLDER libraries)

if(AA_BUILD_TESTS)
	add_tester_scripts(graphtest SOURCES graphio.script)
endif()