add_library(heap heap.c heap.h)
target_link_libraries(heap memdb)
target_include_directories(heap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(heap PROPERTIES FOLDER libraries)

if(AA_BUILD_TESTS)
	add_tester_module(heaptest SOURCES heap.test.c LINKS heap)
	add_tester_scripts(heaptest SOURCES heap.script)
endif()

if(AA_PYTHON_SUPPORT)
	add_subdirectory(python)
endif()