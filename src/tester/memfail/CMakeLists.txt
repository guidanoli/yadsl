add_executable(memfail memfail.c)
target_link_libraries(memfail yadsl)
set_target_properties(memfail PROPERTIES FOLDER tests)
