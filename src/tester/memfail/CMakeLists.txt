add_executable(memfail memfail.c)
target_link_libraries(memfail yadsl)
target_include_directories(memfail PUBLIC ${YADSL_SOURCE_DIR})
set_target_properties(memfail PROPERTIES FOLDER tests)
