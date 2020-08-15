cmake_minimum_required(VERSION 3.0)

# Adds python module (wrapper around add_library)
# add_python_module(target modulename source1 [source2 ...])
function(add_python_module target modulename)
	add_library(${target} SHARED ${ARGN})
	set_target_properties(
		${target}
		PROPERTIES
			PREFIX ""
			DEBUG_POSTFIX "_d"
			OUTPUT_NAME "${modulename}"
			LINKER_LANGUAGE C
	)
	if(WIN32)
		set_target_properties(
			${target}
			PROPERTIES
			SUFFIX ".pyd"
		)
	endif()
	target_include_directories(${target} PUBLIC ${PYTHON_INCLUDE_DIRS})
	# On Windows, it is required to link to the Python libraries
	if(WIN32)
		target_link_libraries(${target} ${PYTHON_LIBRARIES} aa)
	endif()
	set_target_properties(${target} PROPERTIES FOLDER python)
	
	include(moveLibraries)
	move_target_library(${target})
endfunction()
