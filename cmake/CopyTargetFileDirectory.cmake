cmake_minimum_required(VERSION 3.0)

string(CONCAT YADSL_LIBDIR
	"${PROJECT_SOURCE_DIR}/lib"                     # /lib folder
	"$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},4>,32,64>" # Architecture (32-bit or 64-bit binaries)
	"$<IF:$<CONFIG:Debug>,d,>")                     # Configuration (Debug or Release)

set(YADSL_LIBDIR ${YADSL_LIBDIR} PARENT_SCOPE)

# Copy target file directory to appropriate folder
# copy_target_file_directory(mylib)
function(copy_target_file_directory target)
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			"$<TARGET_FILE_DIR:${target}>"
			"${YADSL_LIBDIR}")
endfunction()
