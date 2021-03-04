add_yadsl_library(testerutils TEST STATIC
   SOURCES testerutils.c testerutils.h)
target_link_libraries(testerutils tester string)