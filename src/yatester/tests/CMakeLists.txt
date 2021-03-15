# Add yatester module and all scripts too
# Gets basename to name tester executable
# Eg.: source = empty.test.c => name = empty => testername = emptytest
# Adds all scripts that match $name.*.script
# Adds all failing scripts that match $name.*.fail
# add_yatester_selftest(source)
function(add_yatester_selftest source)
	get_filename_component(source_we ${source} NAME_WE)
	string(CONCAT name ${source_we} test)
	add_executable(${name} ${source})
	target_link_libraries(${name} yatester)
	set_target_properties(${name} PROPERTIES FOLDER tests)
	string(CONCAT script_regex ${source_we} ".*.script")
	file(GLOB SCRIPTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${script_regex})
	if(SCRIPTS)
		add_tester_scripts(${name} SOURCES ${SCRIPTS})
	endif()
	string(CONCAT failing_script_regex ${source_we} ".*.fail")
	file(GLOB FAILING_SCRIPTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${failing_script_regex})
	if(FAILING_SCRIPTS)
		add_tester_scripts(${name} SOURCES ${FAILING_SCRIPTS})
		set_tests_properties(${FAILING_SCRIPTS} PROPERTIES WILL_FAIL TRUE)
	endif()
endfunction()

# Add failing test with explicit command
# add_failing_test(NAME <name> COMMAND <command...>)
function(add_failing_test)
	set(multiValueArgs NAME COMMAND)
	cmake_parse_arguments(ARG "" "" "${multiValueArgs}" ${ARGN})
	if(NOT DEFINED ARG_NAME)
		message(FATAL_ERROR "NAME not defined")
	endif()
	if(NOT DEFINED ARG_COMMAND)
		message(FATAL_ERROR "COMMAND not defined")
	endif()
	add_test(NAME ${ARG_NAME} COMMAND ${ARG_COMMAND})
	set_tests_properties(${ARG_NAME} PROPERTIES WILL_FAIL TRUE)
endfunction()

add_yatester_selftest(empty.test.c)
add_failing_test(NAME empty.1.fail COMMAND emptytest "--list-commands")
add_failing_test(NAME empty.2.fail COMMAND emptytest "--input-file" "x")

if(CMAKE_BUILD_TYPE STREQUAL Debug)
	add_failing_test(NAME empty.3.fail COMMAND emptytest "--log-file" "/")
	add_failing_test(NAME empty.4.fail COMMAND emptytest "--enable-log-channel" "x")
	add_failing_test(NAME empty.5.fail COMMAND emptytest "--malloc-failing-rate" "x")
	add_failing_test(NAME empty.6.fail COMMAND emptytest "--malloc-failing-index" "x")
	add_failing_test(NAME empty.7.fail COMMAND emptytest "--prng-seed" "x")
endif()

add_yatester_selftest(self.test.c)
