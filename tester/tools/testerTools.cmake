cmake_minimum_required(VERSION 3.0)

# Add tester module
# add_tester_module(<name> SOURCES source1 [source2 ...]
#					[LINKS link1 [link2 ...]
#					[DEFINE_SYMBOL external])
function(add_tester_module name)
	set(oneValueArgs DEFINE_SYMBOL)
	set(multiValueArgs SOURCES LINKS)
	cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	add_executable(${name} ${ARG_SOURCES} ${ARG_DEFINE_SYMBOL})
	if(DEFINED ARG_DEFINE_SYMBOL)
		target_compile_definitions(${name} PUBLIC
		TESTER_EXTERNAL_RETURN_VALUES="${ARG_DEFINE_SYMBOL}")
	endif()
	foreach(LINK ${ARG_LINKS})
		if (NOT TARGET ${LINK})
			message(FATAL_ERROR "'${LINK}' is not a target")
		endif()
	endforeach()
	target_link_libraries(${name} ${ARG_LINKS} tester)
endfunction()

# Add a tester script
# add_tester_scripts(target script1 [script2 ...])
function (add_tester_script target)
	set(multiValueArgs SOURCES)
	cmake_parse_arguments(ARG "" "" "${multiValueArgs}" ${ARGN})
	if(NOT TARGET ${target})
		message(FATAL_ERROR "First argument must be TARGET")
	elseif(NOT DEFINED ARG_SOURCES)
		message(FATAL_ERROR "SOURCES not defined")
	endif()
	foreach(SCRIPT ${ARG_SOURCES})
		get_filename_component(SCRIPT_PATH ${SCRIPT} ABSOLUTE)
		add_test(NAME ${SCRIPT} COMMAND ${target} ${SCRIPT_PATH})
	endforeach()
endfunction()