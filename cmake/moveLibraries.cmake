cmake_minimum_required(VERSION 3.0)

# Move target libraries to appropriate output folder
# move_target_library(mylib)
function(move_target_library target)
	string(CONCAT output_folder
		"${PROJECT_SOURCE_DIR}/lib"                     # /lib folder
		"$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},4>,32,64>" # Architecture (32-bit or 64-bit binaries)
		"$<IF:$<CONFIG:Debug>,d,>")                     # Configuration (Debug or Release)
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			"$<TARGET_FILE_DIR:${target}>"
			"${output_folder}")
endfunction()
