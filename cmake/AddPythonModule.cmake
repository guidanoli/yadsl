cmake_minimum_required(VERSION 3.0)

# Adds python module (wrapper around add_library)
# add_python_module(target modulename source1 [source2 ...])
function(add_python_module target modulename)
	add_library(${target} SHARED ${ARGN})
	set_target_properties(
		${target}
		PROPERTIES
			PREFIX ""
			SUFFIX "${PYTHON_EXT_SUFFIX}"
			OUTPUT_NAME "${modulename}"
			LINKER_LANGUAGE C
	)
	target_include_directories(${target} PUBLIC ${PYTHON_INCLUDE_DIRS})
	# On Windows, it is required to link to the Python libraries
	if(WIN32)
		target_link_libraries(${target} ${PYTHON_LIBRARIES})
	endif()
	set_target_properties(${target} PROPERTIES FOLDER python)
	
	include(CopyTargetFileDirectory)
	copy_target_file_directory(${target})
endfunction()
