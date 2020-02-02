add_library(tester tester.c tester.h)
target_include_directories(tester PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Makes tester tools visible to global scope and allows tools to
# be included by other projects too.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/tools")
include(testerTools)

add_tester_module(testertest SOURCES tester.test.c)
add_tester_scripts(testertest SOURCES tester.script)