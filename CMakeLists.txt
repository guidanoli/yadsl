cmake_minimum_required(VERSION 3.12)
project(yadsl)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(YADSL_BUILD_TESTS "Build unit tests" ON)
option(YADSL_BUILD_ALL_TESTS "Build all unit tests" OFF)
option(YADSL_BUILD_LONG_TESTS "Build long unit tests" OFF)
option(YADSL_BUILD_SELF_TESTS "Build self tests" OFF)
option(YADSL_PYTHON_SUPPORT "Build Python C extension modules" OFF)
option(YADSL_LUA_SUPPORT "Build Lua C bindings" OFF)
option(YADSL_GENERATE_DOCS "Generate documentation" OFF)

if(YADSL_BUILD_TESTS AND YADSL_BUILD_ALL_TESTS)
	set(YADSL_BUILD_LONG_TESTS ON)
	set(YADSL_BUILD_SELF_TESTS ON)
endif()

# Allow hierarchical solution
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Add cmake scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(YADSL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src"
    CACHE INTERNAL "YADSL source directory")

# Add 'add_yadsl_library' function
include(AddYADSLLibrary)

if(YADSL_BUILD_TESTS)
	enable_testing()
endif()

if(YADSL_PYTHON_SUPPORT)
	# Find Python interpreter and library
	find_package(Python 3.6 REQUIRED COMPONENTS Interpreter Development)

	# Get Python extension module suffix
	execute_process(
		COMMAND "${Python_EXECUTABLE}" "${YADSL_SOURCE_DIR}/yadsl/ext.py"
		OUTPUT_VARIABLE PYTHON_EXT_SUFFIX
		RESULT_VARIABLE PYTHON_EXT_SUFFIX_NOT_FOUND) 

	if(PYTHON_EXT_SUFFIX_NOT_FOUND)
		message(FATAL_ERROR "Could not obtain Python extension module suffix")
	endif()
	
	# Add 'add_python_module' function
	include(AddPythonModule)
endif()

find_package(Lua 5.4 MODULE)

if (YADSL_LUA_SUPPORT)
	if (NOT LUA_FOUND)
		message(FATAL_ERROR "Lua supported but not found")
	endif()

	# Add 'add_lua_module' function
	include(AddLuaModule)
endif()

if (YADSL_GENERATE_DOCS)
    add_subdirectory(docs)
endif()

# Search for math library
include(CheckCSourceCompiles)
set(LIBM_TEST_SOURCE "#include<math.h>\nfloat f; int main(){sqrt(f);return 0;}")
check_c_source_compiles("${LIBM_TEST_SOURCE}" HAVE_MATH)
if(HAVE_MATH)
  set(LIBM_LIBRARIES)
else()
  set(CMAKE_REQUIRED_LIBRARIES m)
  check_c_source_compiles("${LIBM_TEST_SOURCE}" HAVE_LIBM_MATH)
  unset(CMAKE_REQUIRED_LIBRARIES)
  if(NOT HAVE_LIBM_MATH)
    message(FATAL_ERROR "Unable to use C math library functions")
  endif()
  set(LIBM_LIBRARIES m)
endif()

add_subdirectory(src)
