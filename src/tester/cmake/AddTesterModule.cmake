cmake_minimum_required(VERSION 3.0)

# Add tester module
# add_tester_module(<name> SOURCES source1 [source2 ...]
#                   [LINKS link1 [link2 ...])
function(add_tester_module name)
	set(multiValueArgs SOURCES LINKS)
	cmake_parse_arguments(ARG "" "" "${multiValueArgs}" ${ARGN})
	add_executable(${name} ${ARG_SOURCES})
	target_link_libraries(${name} ${ARG_LINKS} tester)
	set_target_properties(${name} PROPERTIES FOLDER tests)
endfunction()
