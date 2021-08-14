cmake_minimum_required(VERSION 3.0)

# Add a tester script
# add_tester_scripts(target SOURCES script1 [script2 ...])
function(add_tester_scripts target)
	set(multiValueArgs SOURCES)
	cmake_parse_arguments(ARG "" "" "${multiValueArgs}" ${ARGN})
	if(NOT DEFINED ARG_SOURCES)
		message(FATAL_ERROR "SOURCES not defined")
	endif()
	foreach(SCRIPT ${ARG_SOURCES})
		get_filename_component(SCRIPT_PATH ${SCRIPT} ABSOLUTE)
		add_test(NAME ${SCRIPT}
			COMMAND ${target}
			"--input-file" ${SCRIPT_PATH})
		if(CMAKE_BUILD_TYPE MATCHES Debug)
			if(YADSL_BUILD_LONG_TESTS)
				add_test(NAME "${SCRIPT}_memfail"
					COMMAND memfail $<TARGET_FILE:${target}> ${SCRIPT_PATH}
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
			endif()
		endif()
	endforeach()
endfunction()
