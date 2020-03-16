add_library(map map.c map.h)
target_link_libraries(map set aa memdb)
target_include_directories(map PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(map PROPERTIES FOLDER libraries)

if(AA_BUILD_TESTS)
	add_tester_module(maptest SOURCES map.test.c LINKS map testerutils)
	add_tester_scripts(maptest SOURCES map.script)
endif()