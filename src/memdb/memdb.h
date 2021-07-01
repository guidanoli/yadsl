#ifndef __YADSL_MEMDB_H__
#define __YADSL_MEMDB_H__

/**
* \defgroup memdb Memory Debugger
 * @brief A handy tool for spotting memory leaks with little to no effort.
 *
 * The Memory Debuger allows further investigation in cases of memory
 * leakage due to unresponsible housekeeping. It overwrites the main
 * dynamic allocation routines in order to keep track of the blocks
 * that are allocated and dealocatted in and from the heap.
 *
 * The idea is that, at the end of your program, the list of allocated
 * memory blocks should be empty. Otherwise, this indicates that some memory
 * block was not properly deallocated from the heap.
 *
 * @{
*/

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

/**
 * @brief Log channels
*/
typedef enum
{
	YADSL_MEMDB_LOG_CHANNEL_ALLOCATION, /**< When allocating memory */
	YADSL_MEMDB_LOG_CHANNEL_DEALLOCATION, /**< When deallocating memory */
	YADSL_MEMDB_LOG_CHANNEL_LEAKAGE, /**< When memory leak is detected */
}
yadsl_MemDebugLogChannel;

/**
 * @brief List of allocated memory blocks (AMB)
*/
struct yadsl_MemDebugAMB_s
{
	struct yadsl_MemDebugAMB_s* next; /**< Next node (nullable) */
	const char* funcname; /**< Name of function called to allocate it */
	const char* file; /**< File where function was called */
	int line; /**< Line where function was called, in file */
	size_t size; /**< Size of memory block */
	void* amb; /**< Pointer to memory block (unique) */
};

typedef struct yadsl_MemDebugAMB_s yadsl_MemDebugAMB;

/**
 * @brief Get allocated memory block list
 * @ return list or NULL if list is empty
 */
yadsl_MemDebugAMB*
yadsl_memdb_get_amb_list();

/**
 * @brief Check if log channels is enabled
 * @param log_channel log channel
 * @return whether log channel is enabled or not
*/
bool
yadsl_memdb_log_channel_get(
		yadsl_MemDebugLogChannel log_channel);

/**
 * @brief Set log channels enabled or disabled
 * @param log_channel log channel
 * @param enable enable/disable boolean
*/
void
yadsl_memdb_log_channel_set(
		yadsl_MemDebugLogChannel log_channel,
		bool enable);

/**
 * @brief Get (re)allocation fail rate
 * @return percentage (0.0 - 1.0)
*/
float
yadsl_memdb_get_fail_rate();

/**
 * @brief Set (re)allocation fail rate
 * @param fail_rate percentage
*/
void
yadsl_memdb_set_fail_rate(
		float fail_rate);

/**
 * @brief Get whether fial by index is enabled or not
 * @return enabled (true) or disabled (false)
*/
bool
yadsl_memdb_get_fail_by_index();

/**
 * @brief Fail by (re)allocation index
 * @param fail_by_index enable (true) or disable (false)
*/
void
yadsl_memdb_set_fail_by_index(
		bool fail_by_index);

/**
 * @brief Get (re)allocation fail index
 * @return index
*/
size_t
yadsl_memdb_get_fail_index();

/**
 * @brief Set (re)allocation fail index
 * @param fail_index index
*/
void
yadsl_memdb_set_fail_index(
		size_t fail_index);

/**
 * @brief Get size of list of allocated memory blocks
 * @return size of list
*/
size_t
yadsl_memdb_amb_list_size();

/**
 * @brief Check if data is contained in list
 * @param mem data pointer
 * @return whether data is in the list or not
*/
bool
yadsl_memdb_contains_amb(
		void* mem);

/**
 * @brief Set PRNG seed
 * @param seed PRNG seed
*/
void
yadsl_memdb_set_prng_seed(
		unsigned int seed);

/**
 * @brief Check if an error has occurred
 * @return whether error occurred (true) or not (false)
*/
bool
yadsl_memdb_error_occurred();

/**
 * @brief Check if an allocation fail has occurred
 * @return whether fail occurred (true) or not (false)
*/
bool
yadsl_memdb_fail_occurred();

/**
 * @brief Clear list of allocated memory blocks
*/
void
yadsl_memdb_clear_amb_list();

/**
 * @brief Clear list of allocated memory blocks from file
 * @param file file name (__FILE__)
*/
void
yadsl_memdb_clear_amb_list_from_file(const char* file);

/**
 * @brief Set logging output to file pointer
 *
 * Hint
 * ----
 * Setting it to `NULL` unsets the logging output.
 *
 * @param fp file pointer
*/
void
yadsl_memdb_set_logger(
		FILE* fp);

/**
 * @brief Dump memory space information
*/
void
yadsl_memdb_dump(
		FILE* fp,
		void* mem);

/* stdlib.h wrapped functions */

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

#if !defined(YADSL_MEMDB_DONT_DEFINE_MACROS) && (defined(YADSL_RELEASE) || defined(YADSL_DEBUG))
#  define yadsl_memdb_status() printf("MEMDB: %zu items in list\n", yadsl_memdb_amb_list_size())
#  define free yadsl_memdb_free
#  define malloc(size) yadsl_memdb_malloc(size, __FILE__, __LINE__)
#  define realloc(mem, size) yadsl_memdb_realloc(mem, size, __FILE__, __LINE__)
#  define calloc(cnt, size) yadsl_memdb_calloc(cnt, size, __FILE__, __LINE__)
#endif /* YADSL_MEMDB_DONT_DEFINE_MACROS */

/** @} */

#endif
