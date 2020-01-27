add_library(set set.c set.h)
add_executable(settest set.test.c)
target_link_libraries(settest set var)
target_include_directories(set PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})