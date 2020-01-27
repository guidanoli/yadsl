add_library(graphsearch graphsearch.c graphsearch.h)
target_link_libraries(graphsearch graph map)
target_include_directories(graphsearch PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})