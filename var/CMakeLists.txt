add_library(var var.c var.h)
add_executable(vartest var.test.c)
target_link_libraries(vartest var)
target_include_directories(var PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})