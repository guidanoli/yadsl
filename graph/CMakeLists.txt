cmake_minimum_required(VERSION 3.0)
project(graphlib VERSION 1.0.0 DESCRIPTION "Simple graph library in C")

include(GNUInstallDirs)

# For Windows compatibility
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(INCLUDE_DIR include)
set(SOURCE_DIR src)
set(TESTS_DIR tests)
set(CVERSION 99)

############################################################
# Variable library
############################################################

add_library(varlib SHARED
	${SOURCE_DIR}/var.c
	${INCLUDE_DIR}/var.h)
target_include_directories(varlib PRIVATE ${INCLUDE_DIR})
set_target_properties(varlib PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	C_STANDARD ${CVERSION}
	PUBLIC_HEADER ${INCLUDE_DIR}/var.h)

############################################################
# Set library
############################################################

add_library(setlib SHARED
	${SOURCE_DIR}/set.c
	${INCLUDE_DIR}/set.h)
target_include_directories(setlib PRIVATE ${INCLUDE_DIR})
set_target_properties(setlib PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 3
	C_STANDARD ${CVERSION}
	PUBLIC_HEADER ${INCLUDE_DIR}/set.h)

############################################################
# Map library
############################################################

add_library(maplib SHARED
	${SOURCE_DIR}/map.c
	${INCLUDE_DIR}/map.h
	${INCLUDE_DIR}/set.h)
target_include_directories(maplib PRIVATE ${INCLUDE_DIR})
target_link_libraries(maplib setlib)
set_target_properties(maplib PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	C_STANDARD ${CVERSION}
	PUBLIC_HEADER ${INCLUDE_DIR}/map.h)

############################################################
# Graph library
############################################################

add_library(graphlib SHARED
	${SOURCE_DIR}/graph.c
	${INCLUDE_DIR}/graph.h
	${INCLUDE_DIR}/set.h)
target_include_directories(graphlib PRIVATE ${INCLUDE_DIR})
target_link_libraries(graphlib setlib)
set_target_properties(graphlib PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 2
	C_STANDARD ${CVERSION}
	PUBLIC_HEADER ${INCLUDE_DIR}/graph.h)

############################################################
# Graph IO library
############################################################

add_library(graphiolib SHARED
	${SOURCE_DIR}/graphio.c
	${INCLUDE_DIR}/graphio.h
	${INCLUDE_DIR}/graph.h
	${INCLUDE_DIR}/map.h)
target_include_directories(graphiolib PRIVATE ${INCLUDE_DIR})
target_link_libraries(graphiolib graphlib maplib)
set_target_properties(graphiolib PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	C_STANDARD ${CVERSION}
	PUBLIC_HEADER ${INCLUDE_DIR}/graphio.h)

############################################################
# Variable test module
############################################################

add_executable(vartest ${TESTS_DIR}/var.test.c)
target_include_directories(vartest PRIVATE ${INCLUDE_DIR})
target_link_libraries(vartest varlib)
set_target_properties(vartest PROPERTIES C_STANDARD ${CVERSION})

############################################################
# Set test module
############################################################

add_executable(settest ${TESTS_DIR}/set.test.c)
target_include_directories(settest PRIVATE ${INCLUDE_DIR})
target_link_libraries(settest setlib varlib)
set_target_properties(settest PROPERTIES C_STANDARD ${CVERSION})

############################################################
# Graph test module
############################################################

add_executable(graphtest ${TESTS_DIR}/graph.test.c)
target_include_directories(graphtest PRIVATE ${INCLUDE_DIR})
target_link_libraries(graphtest graphlib graphiolib varlib)
set_target_properties(graphtest PROPERTIES C_STANDARD ${CVERSION})

############################################################
# Map test module
############################################################

add_executable(maptest ${TESTS_DIR}/map.test.c)
target_include_directories(maptest PRIVATE ${INCLUDE_DIR})
target_link_libraries(maptest maplib varlib)
set_target_properties(maptest PROPERTIES C_STANDARD ${CVERSION})

############################################################
# pkg-config file
############################################################

configure_file(graphlib.pc.in graphlib.pc @ONLY)

############################################################
# Installation
############################################################

install(FILES
	${CMAKE_BINARY_DIR}/mylib.pc
	DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

install(TARGETS graphlib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
