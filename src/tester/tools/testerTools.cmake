cmake_minimum_required(VERSION 3.0)

# Add tester module
# add_tester_module(<name> SOURCES source1 [source2 ...]
#					[LINKS link1 [link2 ...])
function(add_tester_module name)
	set(multiValueArgs SOURCES LINKS)
	cmake_parse_arguments(ARG "" "" "${multiValueArgs}" ${ARGN})
	add_executable(${name} ${ARG_SOURCES})
	target_link_libraries(${name} ${ARG_LINKS} tester)
	set_target_properties(${name} PROPERTIES FOLDER tests)
endfunction()

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
		add_test(NAME "${SCRIPT} (leaking %0) " COMMAND
			${target} "--input-file" ${SCRIPT_PATH}
			"--enable-log-channel" "LEAKAGE")
		if(YADSL_BUILD_LONG_TESTS)
			set(seedmax 99)
		else()
			set(seedmax 0)
		endif()
		foreach(seed RANGE ${seedmax})
			add_test(NAME "${SCRIPT} (leaking 1%, seed ${seed})" COMMAND
				${target} "--input-file" ${SCRIPT_PATH}
				"--enable-log-channel" "ALLOCATION"
				"--enable-log-channel" "DEALLOCATION"
				"--enable-log-channel" "LEAKAGE"
				"--malloc-failing-rate" "0.01"
				"--prng-seed" "${seed}")
		endforeach()
	endforeach()
endfunction()