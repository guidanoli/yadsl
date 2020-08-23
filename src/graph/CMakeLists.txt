add_yadsl_library(graph TEST PYTHON
    SOURCES graph.c graph.h
    TEST_LINKS graphio graphsearch)
target_link_libraries(graph set)