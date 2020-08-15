cmake_minimum_required(VERSION 3.0)

# Links libraries to target, always checking if
# they are not equal (STREQUAL)
#
# safe_target_link_libraries(target [tgt1 ...])
#
function(safe_target_link_libraries target)
    foreach(arg IN LISTS ARGN)
        if(NOT arg STREQUAL target)
            target_link_libraries(${target} ${arg})
        endif()
    endforeach()
endfunction()

# Adds library for aa (wrapper around add_library)
#
# add_aa_library(target [TEST] [LUA] [PYTHON] [STATIC]
#                SOURCES src1 [src2 ...]
#                TEST_LINKS tgt1 [tgt2 ...])
#
function(add_aa_library target)
    set(options TEST PYTHON LUA STATIC)
    set(oneValueArgs)
    set(multiValueArgs SOURCES TEST_LINKS)
    
    cmake_parse_arguments(OPTS "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN})
    
    add_library(${target} ${OPTS_SOURCES})
    safe_target_link_libraries(${target} aa memdb)
    target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    
    set_target_properties(${target} PROPERTIES
        FOLDER libraries)
    
    if(NOT ${STATIC})
        set_target_properties(${target} PROPERTIES
            POSITION_INDEPENDENT_CODE ON)
    endif()

    include(moveLibraries)
	move_target_library(${target})
    
    if(${OPTS_TEST} AND ${AA_BUILD_TESTS})
        list(APPEND OPTS_TEST_LINKS "${target};testerutils")
        add_tester_module("${target}test" SOURCES "${target}.test.c" LINKS ${OPTS_TEST_LINKS})
        add_tester_scripts("${target}test" SOURCES "${target}.script")
    endif()

    if(${OPTS_PYTHON} AND ${AA_PYTHON_SUPPORT})
        add_subdirectory(python)
    endif()
    
    if (${OPTS_LUA} AND ${AA_LUA_SUPPORT})
        add_subdirectory(lua)
    endif()
endfunction()