#ifndef __YADSL_MEMDB_H__
#define __YADSL_MEMDB_H__

/**
 * \defgroup memdb Memory Debugger
 * @brief The memory debugger provides a common interface for
 * all the allocation, reallocation and deallocation of dynamic
 * memory space and registration of listeners.
 * @{
*/

#include <stddef.h>
#include <stdbool.h>

typedef enum
{
	YADSL_MEMDB_FREE,
	YADSL_MEMDB_MALLOC,
	YADSL_MEMDB_REALLOC,
	YADSL_MEMDB_CALLOC,
	YADSL_MEMDB_FUNCTION_MAX, /* For debugging */
}
yadsl_MemDebugFunction;

typedef struct
{
	const void* ptr;
}
yadsl_MemDebugFreeEvent;

typedef struct
{
	size_t size;
}
yadsl_MemDebugMallocEvent;

typedef struct
{
	const void* ptr;
	size_t size;
}
yadsl_MemDebugReallocEvent;

typedef struct
{
	size_t nmemb;
	size_t size;
}
yadsl_MemDebugCallocEvent;

typedef struct
{
	yadsl_MemDebugFunction function;
	const char* file;
	int line;
	union
	{
		yadsl_MemDebugFreeEvent free;
		yadsl_MemDebugMallocEvent malloc;
		yadsl_MemDebugReallocEvent realloc;
		yadsl_MemDebugCallocEvent calloc;
	};
}
yadsl_MemDebugEvent;

typedef void yadsl_MemDebugListenerHandle;
typedef void yadsl_MemDebugListenerArgument;
typedef bool (*yadsl_MemDebugAcceptCallback)(yadsl_MemDebugEvent const* event, yadsl_MemDebugListenerArgument* arg);
typedef void (*yadsl_MemDebugAcknowledgeCallback)(yadsl_MemDebugEvent const* event, const void* ptr, yadsl_MemDebugListenerArgument* arg);

/**
 * @brief Add a listener to the memory debugger
 * @param accept_cb an acceptance callback
 * @param ack_cb an acknowledge callback
 * @param arg an auxiliary argument
 * @return a listener handle or NULL if could not
 * allocate enough memory for a listener
*/
yadsl_MemDebugListenerHandle*
yadsl_memdb_add_listener(
	yadsl_MemDebugAcceptCallback accept_cb,
	yadsl_MemDebugAcknowledgeCallback ack_cb,
	yadsl_MemDebugListenerArgument* arg);

/**
 * @brief Remove a listener from the memory debugger
 * @param handle listener handle
 * @return whether the listener was previously added
 * and was now removed by this call
*/
bool
yadsl_memdb_remove_listener(
	yadsl_MemDebugListenerHandle* handle);

/**
 * @brief Wrapper around free
*/
void
yadsl_memdb_free(
	void* ptr,
	const char* file,
	int line);

/**
 * @brief Wrapper around malloc
*/
void*
yadsl_memdb_malloc(
	size_t size,
	const char* file,
	int line);

/**
 * @brief Wrapper around realloc
*/
void*
yadsl_memdb_realloc(
	void* ptr,
	size_t size,
	const char* file,
	int line);

/**
 * @brief Wrapper around calloc
*/
void*
yadsl_memdb_calloc(
	size_t nmemb,
	size_t size,
	const char* file,
	int line);

/** @} */

#endif
