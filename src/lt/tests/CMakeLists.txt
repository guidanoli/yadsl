# Add an lt script
# Optionally, you can specify a passing regex (default: "")
# and whether the script is going to fail (default: false)
# and even the test name (default: script path)
# add_lt_script(script [REGEX regex] [WILL_FAIL] [NAME name])
function(add_lt_test script)
	set(options WILL_FAIL)
	set(oneValueArgs REGEX NAME)
	cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "" ${ARGN})

	get_filename_component(script_path "${script}" ABSOLUTE)

	if(ARG_NAME)
		set(testname "${ARG_NAME}")
	else()
		file(RELATIVE_PATH testname "${YADSL_SOURCE_DIR}" "${script_path}")
	endif()
	
	add_test(NAME "${testname}"
	         COMMAND lt "${script_path}"
			 WORKING_DIRECTORY "${YADSL_LIBDIR}")

	if(ARG_WILL_FAIL)
		set_tests_properties("${testname}" PROPERTIES WILL_FAIL "${ARG_WILL_FAIL}")
	endif()

	if(ARG_REGEX)
		add_lt_test("${script}" NAME "${testname}.regex")
		set_tests_properties("${testname}.regex" PROPERTIES PASS_REGULAR_EXPRESSION "${ARG_REGEX}")
	endif()
endfunction()

if(NOT YADSL_BUILD_SELF_TESTS)
	return()
endif()

add_lt_test(return_nil.lua REGEX "test bench must be table, not nil" WILL_FAIL)
add_lt_test(return_number.lua REGEX "test bench must be table, not number" WILL_FAIL)
add_lt_test(return_array.lua REGEX "test case name must be string, not number" WILL_FAIL)
add_lt_test(return_table_with_numbers.lua REGEX "test case must be function, not number" WILL_FAIL)
add_lt_test(return_table.lua REGEX "mytest \\.\\.\\. my test called\nok")
add_lt_test(failing_test.lua NAME lt_fail1 REGEX "mytest \\.\\.\\. my test called\nFAIL" WILL_FAIL)
add_lt_test(failing_test.lua NAME lt_fail2 REGEX "FAIL: mytest" WILL_FAIL)
add_lt_test(syntax_error.lua REGEX "unexpected symbol" WILL_FAIL)
add_lt_test(cannot_open.lua REGEX "cannot open" WILL_FAIL)
add_lt_test("${CMAKE_CURRENT_SOURCE_DIR}" REGEX "cannot read" WILL_FAIL) # directory
add_lt_test(empty.lua REGEX "hello world")
add_lt_test(runtime_error.lua REGEX "my error message" WILL_FAIL)
add_lt_test(ok_fail.lua NAME lt_ok_fail1 REGEX "ok \\.\\.\\. ok called\nok" WILL_FAIL)
add_lt_test(ok_fail.lua NAME lt_ok_fail2 REGEX "fail \\.\\.\\. fail also called\nFAIL" WILL_FAIL)
add_lt_test(ok_fail.lua NAME lt_ok_fail3 REGEX "FAIL: fail" WILL_FAIL)
add_lt_test(ok_fail.lua NAME lt_ok_fail4 REGEX "my error message" WILL_FAIL)
