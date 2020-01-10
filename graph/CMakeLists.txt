cmake_minimum_required(VERSION 3.0)
project(graphlib VERSION 0.0.2 DESCRIPTION "Simple graph library in C")

include(GNUInstallDirs)

set(INCLUDE_DIR include)
set(SOURCE_DIR src)
set(TESTS_DIR tests)
set(SOURCES ${SOURCE_DIR}/graph.c ${SOURCE_DIR}/set.c)
set(HEADERS ${INCLUDE_DIR}/graph.h ${INCLUDE_DIR}/set.h)
set(CVERSION 99)

add_library(graphlib ${SOURCES} ${HEADERS})
target_include_directories(graphlib PRIVATE ${INCLUDE_DIR})
set_target_properties(graphlib PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 2
	C_STANDARD ${CVERSION}
	PUBLIC_HEADER ${INCLUDE_DIR}/graph.h)

add_executable(graphtest ${TESTS_DIR}/graph.test.c)
target_include_directories(graphtest PRIVATE ${INCLUDE_DIR})
target_link_libraries(graphtest graphlib)
set_target_properties(graphtest PROPERTIES C_STANDARD ${CVERSION})

add_executable(settest ${TESTS_DIR}/set.test.c)
target_include_directories(settest PRIVATE ${INCLUDE_DIR})
target_link_libraries(settest graphlib)
set_target_properties(settest PROPERTIES C_STANDARD ${CVERSION})

configure_file(graphlib.pc.in graphlib.pc @ONLY)

install(TARGETS graphlib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${CMAKE_BINARY_DIR}/mylib.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)