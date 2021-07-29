add_library(yadsl INTERFACE)
target_include_directories(yadsl INTERFACE ${YADSL_SOURCE_DIR})
target_sources(yadsl INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/dll.h
	${CMAKE_CURRENT_SOURCE_DIR}/py.h
	${CMAKE_CURRENT_SOURCE_DIR}/stdlib.h
	${CMAKE_CURRENT_SOURCE_DIR}/utl.h)
