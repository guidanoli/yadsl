# YADSL interface library
add_library(yadsl INTERFACE)
target_include_directories(yadsl INTERFACE ${YADSL_SOURCE_DIR})
target_sources(yadsl INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/dll.h
	${CMAKE_CURRENT_SOURCE_DIR}/py.h
	${CMAKE_CURRENT_SOURCE_DIR}/stdlib.h
	${CMAKE_CURRENT_SOURCE_DIR}/utl.h)
if(YADSL_CODE_COVERAGE)
	if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
		# Add required flags (GCC & LLVM/Clang)
		target_compile_options(yadsl INTERFACE
			-O0        # no optimization
			-g         # generate debug info
			--coverage # sets all required flags
		)
		if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
			target_link_options(yadsl INTERFACE --coverage)
		else()
			target_link_libraries(yadsl INTERFACE --coverage)
		endif()
	else()
		message(WARNING "Compiler ${CMAKE_C_COMPILER_ID} not supported for coverage test")
	endif()
endif()
