cmake_minimum_required(VERSION 3.0)
project(aa)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(AA_BUILD_TESTS "Build unit tests" ON)
option(AA_PYTHON_SUPPORT "Build Python C extension modules" ON)

if(AA_BUILD_TESTS)
	enable_testing()
endif()

if(AA_PYTHON_SUPPORT)
	set(Python_FIND_VIRTUALENV FIRST)
	find_package(PythonInterp 3.6 REQUIRED)

	# Find Python headers
	exec_program(${PYTHON_EXECUTABLE}
		ARGS "-c \"import sysconfig; print(sysconfig.get_paths()['include'])\""
		OUTPUT_VARIABLE PYTHON_INCLUDE_DIRS
		RETURN_VALUE PYTHON_INCLUDE_DIRS_NOT_FOUND
		)
	if(PYTHON_INCLUDE_DIRS_NOT_FOUND)
		message(FATAL_ERROR "Python headers not found")
	endif()

	# This goes after, since it uses PythonInterp as a hint
	if(WIN32)
		find_package(PythonLibs 3.6 REQUIRED)
	endif()
	
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/python")
endif()

# Search for math library
include(CheckCSourceCompiles)
set(LIBM_TEST_SOURCE "#include<math.h>\nfloat f; int main(){sqrt(f);return
0;}")
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

# Allow hierarchical solution
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Tester framework
add_subdirectory(tester)

# Common headers
add_subdirectory(common)

# Modules
add_subdirectory(diff)
add_subdirectory(graph)
add_subdirectory(graphio)
add_subdirectory(graphsearch)
add_subdirectory(map)
add_subdirectory(set)
add_subdirectory(queue)
add_subdirectory(var)
add_subdirectory(heap)
