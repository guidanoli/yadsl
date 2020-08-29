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

# Do logical ternary based on value and save
# result in variable
#
# do_ternary("new_var" old_var OKAY NOT_OKAY)
#
macro(do_ternary varName varValue trueValue falseValue)
  if(${varValue})
    set(${varName} ${trueValue})
  else()
    set(${varName} ${falseValue})
  endif()  
endmacro()

# Invert value and save it in variable
#
# invert_boolean("new_var" old_var)
#
macro(invert_boolean varName varValue)
  do_ternary(${varName} ${varValue} FALSE TRUE)
endmacro()

# Adds library for yadsl (wrapper around add_library)
#
# add_yadsl_library(target [TEST] [LUA] [PYTHON] [STATIC]
#                SOURCES src1 [src2 ...]
#                TEST_LINKS tgt1 [tgt2 ...])
#
function(add_yadsl_library target)
    set(options TEST PYTHON LUA STATIC)
    set(oneValueArgs)
    set(multiValueArgs SOURCES TEST_LINKS)
    
    cmake_parse_arguments(OPTS "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN})
    
    if(${OPTS_PYTHON} OR ${OPTS_LUA})
        set(${OPTS_STATIC} FALSE)
    endif()
    
    add_library(${target} ${OPTS_SOURCES})
    safe_target_link_libraries(${target} yadsl memdb)
    target_include_directories(${target} PUBLIC ${YADSL_SOURCE_DIR})
    
    invert_boolean("OPTS_FPIC" OPTS_STATIC)
        
    set_target_properties(${target} PROPERTIES
        FOLDER libraries
        POSITION_INDEPENDENT_CODE ${OPTS_FPIC})
    
    include(CopyTargetFileDirectory)
	copy_target_file_directory(${target})
    
    if(${OPTS_TEST} AND ${YADSL_BUILD_TESTS})
        list(APPEND OPTS_TEST_LINKS "${target};testerutils")
        add_tester_module("${target}test" SOURCES "${target}.test.c" LINKS ${OPTS_TEST_LINKS})
        add_tester_scripts("${target}test" SOURCES "${target}.script")
    endif()

    if(${OPTS_PYTHON} AND ${YADSL_PYTHON_SUPPORT})
        add_subdirectory(python)
    endif()
    
    if (${OPTS_LUA} AND ${YADSL_LUA_SUPPORT})
        add_subdirectory(lua)
    endif()
endfunction()