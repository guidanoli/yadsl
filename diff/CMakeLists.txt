cmake_minimum_required(VERSION 3.0)
project(difflib VERSION 1.0.0 DESCRIPTION "Simple diff algorithm")

include(GNUInstallDirs)

# For Windows compatibility
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(INCLUDE_DIR include)
set(SOURCE_DIR src)
set(TESTS_DIR tests)
set(CVERSION 99)

############################################################
# Diff library
############################################################

add_library(difflib SHARED
	${SOURCE_DIR}/diff.c
	${INCLUDE_DIR}/diff.h)
target_include_directories(difflib PRIVATE ${INCLUDE_DIR})
set_target_properties(difflib PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	C_STANDARD ${CVERSION}
	PUBLIC_HEADER ${INCLUDE_DIR}/diff.h)

############################################################
# Diff test module
############################################################

add_executable(difftest ${TESTS_DIR}/diff.test.c)
target_include_directories(difftest PRIVATE ${INCLUDE_DIR})
target_link_libraries(difftest difflib)
set_target_properties(difftest PROPERTIES C_STANDARD ${CVERSION})

############################################################
# pkg-config file
############################################################

configure_file(difflib.pc.in difflib.pc @ONLY)

############################################################
# Installation
############################################################

install(FILES
	${CMAKE_BINARY_DIR}/difflib.pc
	DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

install(TARGETS difflib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
