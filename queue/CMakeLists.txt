add_library(queue queue.c queue.h)
add_executable(queuetest queue.test.c)
target_link_libraries(queuetest queue var)
target_include_directories(queue PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})