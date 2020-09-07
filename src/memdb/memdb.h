#ifndef __YADSL_MEMDB_H__
#define __YADSL_MEMDB_H__

/**
* \defgroup memdb Memory Debugger
 * @brief A handy tool for spotting memory leaks with little to no effort.
 *
 * The Memory Debuger allows further investigation in cases of memory
 * leakage due to unresponsible housekeeping. It overwrites the main
 * dynamic allocation routines in order to keep track of the blocks
 * are allocated and dealocatted in and from the heap.
 *
 * The idea is that, at the end of your program, the list of allocated
 * data should be empty. Otherwise, this indicates that some memory
 * block was not properly deallocated from the heap.
 *
 * It is important to include this header AFTER including stdlib.h
 * and string.h in order to correctly override these functions.
 *
 * Also, keep in mind that the macros are only defined when the `_DEBUG` flag
 * is set. Some IDEs, like Visual Studio, already define this flag for Debug builds.
 *
 * @{
*/

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

/**
 * @brief Get size of list of allocated data
 * @return size of list
*/
size_t
yadsl_memdb_list_size();

/**
 * @brief Check if data is contained in list
 * @param _mem data pointer
 * @return whether data is in the list or not
*/
bool
yadsl_memdb_contains(void* _mem);

/**
 * @brief Check if any error occurred
 * @return whether any error occurred
*/
bool
yadsl_memdb_error_occurred();

/**
 * @brief Clear list of allocated data
*/
void
yadsl_memdb_clear_list();

/**
 * @brief Set logging output to file pointer
 *
 * Hint
 * ----
 * Setting it to `NULL` resets the logging output.
 *
 * @param fp file pointer
*/
void
yadsl_memdb_set_logger(FILE* fp);

/* Overriden memory (de)allocation functions */

/**
 * @brief Wrapper around free
*/
void
yadsl_memdb_free(
	void* mem);

/**
 * @brief Wrapper around malloc
*/
void*
yadsl_memdb_malloc(
	size_t size,
	const char* file,
	const int line);

/**
 * @brief Wrapper around realloc
*/
void*
yadsl_memdb_realloc(
	void* mem,
	size_t size,
	const char* file,
	const int line);

/**
 * @brief Wrapper around calloc
*/
void*
yadsl_memdb_calloc(
	size_t cnt,
	size_t size,
	const char* file,
	const int line);

/**
 * @brief Wrapper around strdup
*/
char*
yadsl_memdb_strdup(
	const char* str,
	const char* file,
	const int line);

#ifndef _MEMDB_INTERNAL
#  ifndef _DEBUG
#    define yadsl_memdb_dump() ((void) 0)
#  else
#    define yadsl_memdb_dump() printf("MEMDB: %zu items in list\n", yadsl_memdb_list_size())
#    define free yadsl_memdb_free
#    define malloc(_size) yadsl_memdb_malloc(_size, __FILE__, __LINE__)
#    define realloc(_mem, _size) yadsl_memdb_realloc(_mem, _size, __FILE__, __LINE__)
#    define calloc(_cnt, _size) yadsl_memdb_calloc(_cnt, _size, __FILE__, __LINE__)
#    define strdup(_str) yadsl_memdb_strdup(_str, __FILE__, __LINE__)
#  endif /* _DEBUG */
#endif /* _MEMDB_INTERNAL */

/** @} */

#endif
