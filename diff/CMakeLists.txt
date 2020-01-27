add_library(diff diff.c diff.h)
add_executable(difftest diff.test.c)
target_link_libraries(difftest diff)