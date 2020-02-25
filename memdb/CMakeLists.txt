add_library(memdb memdb.c memdb.h)
target_include_directories(memdb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(memdb PROPERTIES FOLDER libraries)

add_tester_module(memdbtest SOURCES memdb.test.c LINKS memdb)
add_tester_scripts(memdbtest SOURCES memdb.script)
