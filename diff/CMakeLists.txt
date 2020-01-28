add_library(diff diff.c diff.h)
add_executable(difftest diff.test.c)
target_link_libraries(difftest diff)

add_test(NAME EqualStrings COMMAND difftest
   abc abc EQUAL)
add_test(NAME DifferentStrings COMMAND difftest
   abc def DIFFERENT)
add_test(NAME CaseTest COMMAND difftest
   abc aBc DIFFERENT)
add_test(NAME ShiftTest COMMAND difftest
   123 "@!#" DIFFERENT)