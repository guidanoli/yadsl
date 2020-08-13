add_library(hashmap hashmap.c hashmap.h)
target_link_libraries(hashmap memdb)
target_include_directories(hashmap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(hashmap PROPERTIES
    FOLDER libraries
    POSITION_INDEPENDENT_CODE ON)

if(AA_BUILD_TESTS)
	add_tester_module(hashmaptest SOURCES hashmap.test.c
        LINKS hashmap testerutils)
	add_tester_scripts(hashmaptest SOURCES hashmap.script)
endif()

if(AA_PYTHON_SUPPORT)
	add_subdirectory(python)
endif()