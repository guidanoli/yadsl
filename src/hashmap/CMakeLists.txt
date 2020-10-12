add_yadsl_library(hashmap TEST PYTHON
    SOURCES hashmap.c hashmap.h)
target_link_libraries(hashmap string)