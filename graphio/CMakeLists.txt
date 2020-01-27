add_library(graphio graphio.c graphio.h)
target_link_libraries(graphio graph map)
target_include_directories(graphio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})