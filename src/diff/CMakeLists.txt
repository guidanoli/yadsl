add_aa_library(diff TEST PYTHON LUA
    SOURCES diff.c diff.h)
target_link_libraries(diff ${LIBM_LIBRARIES})