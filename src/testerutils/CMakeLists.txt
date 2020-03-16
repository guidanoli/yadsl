add_library(testerutils testerutils.c testerutils.h)
target_link_libraries(testerutils tester)
target_include_directories(testerutils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(testerutils PROPERTIES FOLDER libraries)

if(AA_BUILD_TESTS)
	add_tester_module(testerutilstest SOURCES testerutils.test.c LINKS testerutils)
	add_tester_scripts(testerutilstest SOURCES testerutils.script)
endif()
