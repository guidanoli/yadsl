# Add yatester module
#   source - source file
# add_yatester_module(source)
function(add_yatester_module source)
	get_filename_component(source_we ${source} NAME_WE)
	string(CONCAT name ${source_we} test)
	add_executable(${name} ${source})
	target_link_libraries(${name} yatester)
	set_target_properties(${name} PROPERTIES FOLDER tests)
endfunction()

# Add a test script for a yatester module
#   tgt - target created by add_yatester_module
#   script - script that will be run on the test
#   value - expected return value of the tester
# add_yatester_script(TARGET <tgt> SOURCE <script> [VALUE <value>])
function(add_yatester_script)
	set(one_value_keywords TARGET SOURCE VALUE)
	cmake_parse_arguments(ARG "" "${one_value_keywords}" "" "${ARGN}")
	if(NOT DEFINED ARG_TARGET)
		message(FATAL_ERROR "TARGET not defined")
	endif()
	if(NOT DEFINED ARG_SOURCE)
		message(FATAL_ERROR "SOURCE not defined")
	endif()
	get_filename_component(SOURCE_ABSOLUTE ${ARG_SOURCE} ABSOLUTE)
	set(COMMAND_ARGS ${ARG_TARGET} --input-file "${SOURCE_ABSOLUTE}")
	if(CMAKE_BUILD_TYPE STREQUAL Debug)
		list(APPEND COMMAND_ARGS --enable-log-channel LEAKAGE)
	endif()
	if(DEFINED ARG_VALUE)
		list(APPEND COMMAND_ARGS --expect ${ARG_VALUE})
	endif()
	add_test(NAME ${ARG_SOURCE} COMMAND ${COMMAND_ARGS})
endfunction()

add_yatester_module(empty.test.c)
add_yatester_script(TARGET emptytest SOURCE empty.1.script)
add_yatester_script(TARGET emptytest SOURCE empty.2.script)
add_yatester_script(TARGET emptytest SOURCE empty.3.script)

add_test(NAME empty.1.fail COMMAND emptytest --help --expect 1)
add_test(NAME empty.2.fail COMMAND emptytest --list-commands --expect 1)

# No --expect will fail
add_test(NAME no_expect COMMAND emptytest --list-commands)
set_tests_properties(no_expect PROPERTIES WILL_FAIL TRUE)

# Wrong --expect will fail
add_test(NAME wrong_expect COMMAND emptytest --list-commands --expect 2)
set_tests_properties(wrong_expect PROPERTIES WILL_FAIL TRUE)

add_test(NAME empty.3.fail COMMAND emptytest --input-file x --expect 5)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
	add_test(NAME empty.4.fail COMMAND emptytest --log-file / --expect 5)
	add_test(NAME empty.5.fail COMMAND emptytest --enable-log-channel x --expect 1)
	add_test(NAME empty.6.fail COMMAND emptytest --malloc-failing-rate x --expect 1)
	add_test(NAME empty.7.fail COMMAND emptytest --malloc-failing-index x --expect 1)
	add_test(NAME empty.8.fail COMMAND emptytest --prng-seed x --expect 1)
endif()

add_yatester_module(self.test.c)
add_yatester_script(TARGET selftest SOURCE self.1.script)
add_yatester_script(TARGET selftest SOURCE self.1.fail VALUE 1)
add_yatester_script(TARGET selftest SOURCE self.2.fail VALUE 1)
add_yatester_script(TARGET selftest SOURCE self.3.fail VALUE 6)
add_yatester_script(TARGET selftest SOURCE self.4.fail VALUE 1)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
	add_yatester_script(TARGET selftest SOURCE self.5.fail VALUE 4)
endif()

add_yatester_script(TARGET selftest SOURCE self.6.fail VALUE 6)
add_yatester_script(TARGET selftest SOURCE self.7.fail VALUE 6)
add_yatester_script(TARGET selftest SOURCE self.8.fail VALUE 6)
add_yatester_script(TARGET selftest SOURCE self.9.fail VALUE 6)
add_yatester_script(TARGET selftest SOURCE self.10.fail VALUE 8)
add_yatester_script(TARGET selftest SOURCE self.11.fail VALUE 8)
add_yatester_script(TARGET selftest SOURCE self.12.fail VALUE 1)
add_yatester_script(TARGET selftest SOURCE self.13.fail VALUE 1)
add_yatester_script(TARGET selftest SOURCE self.14.fail VALUE 8)

add_yatester_module(badcommand1.test.c)
add_yatester_script(TARGET badcommand1test SOURCE badcommand1.1.fail VALUE 7)

add_yatester_module(badcommand2.test.c)
add_yatester_script(TARGET badcommand2test SOURCE badcommand2.1.fail VALUE 7)

add_yatester_module(badcommand3.test.c)
add_yatester_script(TARGET badcommand3test SOURCE badcommand3.1.fail VALUE 7)
