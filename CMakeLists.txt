cmake_minimum_required(VERSION 3.0)
project(aa)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(AA_BUILD_TESTS "Build unit tests" ON)
option(AA_PYTHON_SUPPORT "Build Python C extension modules" ON)
option(AA_LUA_SUPPORT "Build Lua C bindings" ON)

# Allow hierarchical solution
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Add cmake scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Add 'add_library' function
include(addLibrary)

if(AA_BUILD_TESTS)
	enable_testing()
endif()

if(AA_PYTHON_SUPPORT)
	# Find Python interpreter
	set(Python_FIND_VIRTUALENV FIRST)
	find_package(PythonInterp 3.6 REQUIRED)

	# Find Python headers
	exec_program(${PYTHON_EXECUTABLE}
		ARGS "-c \"import sysconfig; print(sysconfig.get_paths()['include'])\""
		OUTPUT_VARIABLE PYTHON_INCLUDE_DIRS
		RETURN_VALUE PYTHON_INCLUDE_DIRS_NOT_FOUND)

	if(PYTHON_INCLUDE_DIRS_NOT_FOUND)
		message(FATAL_ERROR "Python headers not found")
	endif()

	# This goes after, since it uses PythonInterp as a hint
	if(WIN32)
		find_package(PythonLibs 3.6 REQUIRED)
	endif()
	
	# Add 'add_python_module' function
	include(addPythonModule)
endif()

if (AA_LUA_SUPPORT)
	find_package(Lua 5.0 REQUIRED MODULE)

	# Add 'add_lua_module' function
	include(addLuaModule)
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
