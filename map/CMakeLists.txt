add_library(map map.c map.h)
target_link_libraries(map set)
target_include_directories(map PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_tester_module(maptest SOURCES map.test.c LINKS map var)
add_tester_scripts(maptest SOURCES map.script)