cmake_minimum_required(VERSION 3.0)

# Adds lua module (wrapper around add_library)
# add_lua_module(target modulename source1 [source2 ...])
function(add_lua_module target modulename)
	add_library(${target} SHARED ${ARGN})
	set_target_properties(
		${target}
		PROPERTIES
			PREFIX ""
			OUTPUT_NAME "${modulename}"
			LINKER_LANGUAGE C
	)
	target_include_directories(${target} PUBLIC ${LUA_INCLUDE_DIR})
	if(WIN32)
		target_link_libraries(${target} ${LUA_LIBRARIES})
	endif()
	set_target_properties(${target} PROPERTIES FOLDER lua)
	
	include(moveLibraries)
	move_target_library(${target})
endfunction()
