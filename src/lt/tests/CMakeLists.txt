# Add an lt script
# Optionally, you can specify passing regexes (default: no regex match)
# and whether the script is going to fail (default: false)
# and even the test name (default: script path)
# add_lt_script(script [PASS_REGULAR_EXPRESSION regex1 regex2 ...]
#                      [FAIL_REGULAR_EXPRESSION regex1 regex2 ...]
#                      [WILL_FAIL] [NAME name])
function(add_lt_test script)
	set(options WILL_FAIL)
	set(oneValueArgs NAME)
	set(multiValueArgs PASS_REGULAR_EXPRESSION FAIL_REGULAR_EXPRESSION)
	cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

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

	set(REGEX_COUNTER 1)

	foreach(REGEX ${ARG_PASS_REGULAR_EXPRESSION})
		set(regex_testname "${testname}/regex/${REGEX_COUNTER}")
		add_lt_test("${script}" NAME "${regex_testname}")
		set_tests_properties("${regex_testname}" PROPERTIES PASS_REGULAR_EXPRESSION "${REGEX}")
		math(EXPR REGEX_COUNTER "${REGEX_COUNTER}+1")
	endforeach()

	foreach(REGEX ${ARG_FAIL_REGULAR_EXPRESSION})
		set(regex_testname "${testname}/regex/${REGEX_COUNTER}")
		add_lt_test("${script}" NAME "${regex_testname}")
		set_tests_properties("${regex_testname}" PROPERTIES FAIL_REGULAR_EXPRESSION "${REGEX}" WILL_FAIL "${ARG_WILL_FAIL}")
		math(EXPR REGEX_COUNTER "${REGEX_COUNTER}+1")
	endforeach()
endfunction()

if(NOT YADSL_BUILD_SELF_TESTS)
	return()
endif()

add_lt_test(return_nil.lua WILL_FAIL
	PASS_REGULAR_EXPRESSION
	"the test script must return a table")

add_lt_test(return_number.lua WILL_FAIL
	PASS_REGULAR_EXPRESSION
	"the test script must return a table")

add_lt_test(return_array.lua
	PASS_REGULAR_EXPRESSION
	"0 failed, 0 passed")

add_lt_test(function_name.lua
	PASS_REGULAR_EXPRESSION
	"0 failed, 0 passed")

add_lt_test(return_table_with_numbers.lua)

add_lt_test(return_table.lua
	PASS_REGULAR_EXPRESSION
	"test \\.\\.\\. my test called\nok"
	"0 failed, 1 passed")

add_lt_test(failing_test.lua WILL_FAIL
	PASS_REGULAR_EXPRESSION
	"test \\.\\.\\. my test called\nFAIL"
	"FAIL: test"
	"1 failed, 0 passed")

add_lt_test(syntax_error.lua WILL_FAIL
	PASS_REGULAR_EXPRESSION
	"unexpected symbol")

add_lt_test(cannot_open.lua WILL_FAIL
	PASS_REGULAR_EXPRESSION
	"cannot open")

add_lt_test("${CMAKE_CURRENT_SOURCE_DIR}" WILL_FAIL
	PASS_REGULAR_EXPRESSION
	"cannot read")

add_lt_test(empty.lua
	PASS_REGULAR_EXPRESSION
	"hello world"
	"0 failed, 0 passed")

add_lt_test(runtime_error.lua WILL_FAIL
	PASS_REGULAR_EXPRESSION
	"my error message")

add_lt_test(ok_fail.lua WILL_FAIL
	PASS_REGULAR_EXPRESSION
	"testok \\.\\.\\. ok called\nok"
	"testfail \\.\\.\\. fail also called\nFAIL"
	"FAIL: testfail"
	"my error message"
	"1 failed, 1 passed")

add_lt_test(check_param.lua
	PASS_REGULAR_EXPRESSION
	"test \\.\\.\\. my test called\nok")
