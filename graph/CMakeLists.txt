cmake_minimum_required(VERSION 3.0)
project(graphlib VERSION 0.0.1 DESCRIPTION "Simple graph library in C")

include(GNUInstallDirs)

add_library(graphlib SHARED src/graph.c src/set.c)
target_include_directories(graphlib PRIVATE include)
set_target_properties(graphlib PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	PUBLIC_HEADER include/graph.h)

configure_file(graphlib.pc.in graphlib.pc @ONLY)

install(TARGETS graphlib
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${CMAKE_BINARY_DIR}/mylib.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)