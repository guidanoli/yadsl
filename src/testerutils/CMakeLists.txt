add_aa_library(testerutils TEST
   SOURCES testerutils.c testerutils.h)
target_link_libraries(testerutils tester)