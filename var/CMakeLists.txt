add_library(var var.c var.h)
add_executable(vartest var.test.c)
target_link_libraries(vartest targp var)
target_include_directories(var PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_test(NAME CreateOne COMMAND vartest
   add 0 text)
add_test(NAME Dump COMMAND vartest
   add 0 text dmp 0)
add_test(NAME SerDeSer COMMAND vartest
   add 0 text w 0 myvar r 1 myvar cmp 0 1 EQUAL)