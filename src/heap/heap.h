#ifndef __YADSL_HEAP_H__
#define __YADSL_HEAP_H__

/**
 * \defgroup heap Heap
 * @brief Generic heap
 *
 * A Heap starts empty and can be populated with arbitrary data
 * ordered by an arbitrary comparison function. Items can be then
 * inserted in or extracted from the heap at any given time.
 *
 * The comparison function must be consistent for any two arbitrary
 * data objects inserted. It will dictate which items must have
 * higher priority over the others.
 *
 * The return of a comparison function should be a boolean value which
 * indicates whether the first item should be higher in the heap than
 * the second item provided.
 *
 * The Heap data structure acquires ownership of an object once it
 * is successfully inserted and looses it if eventually extracted.
 * When destructed, the heap calls an arbitrary free function, given
 * by the caller at time of construction, for every remaining object.
 *
 * @{
*/

#include <stddef.h>

/**
 * @brief Value returned by Heap functions
*/
typedef enum
{
	YADSL_HEAP_RET_OK = 0, /**< All went ok */
	YADSL_HEAP_RET_EMPTY, /**< Heap is empty */
	YADSL_HEAP_RET_FULL, /**< Heap is full */
	YADSL_HEAP_RET_SHRINK, /**< Heap cannot be shrinked */
	YADSL_HEAP_RET_MEMORY, /**< Could not allocate memory */
} yadsl_HeapRet;

typedef void yadsl_HeapHandle; /**< Heap handle */
typedef void yadsl_HeapObj; /**< Heap object (userdata) */
typedef void yadsl_HeapObjCmpArg; /**< Argument passed to yadsl_HeapObjCmpFunc */

/**
 * @brief Function responsible for comparing objects
 * @param obj1 first object
 * @param obj2 second object
 * @param arg user argument
 * @return an integer *n*, where...
 * * *n* > 0 if obj1 > obj2
 * * *n* = 0 if obj1 = obj2
 * * *n* < 0 if obj1 < obj2
*/
typedef int
(*yadsl_HeapObjCmpFunc)(
	yadsl_HeapObj* obj1,
	yadsl_HeapObj* obj2,
	yadsl_HeapObjCmpArg* cmp_objs_arg);

/**
 * @brief Function responsible for freeing objects
 * @param obj object to be freed
*/
typedef void
(*yadsl_HeapObjFreeFunc)(
	yadsl_HeapObj* obj);

/**
 * @brief Create an empty heap
 * @param initial_size heap initial size
 * @param cmp_objs_func object comparison function
 * @param free_obj_func object freeing function
 * @param cmp_objs_arg object comparison function argument
 * @return newly created heap or NULL if could not allocate memory
*/
yadsl_HeapHandle*
yadsl_heap_create(
	size_t initial_size,
	yadsl_HeapObjCmpFunc cmp_objs_func,
	yadsl_HeapObjFreeFunc free_obj_func,
	yadsl_HeapObjCmpArg* cmp_objs_arg);

/**
 * @brief Insert object in heap
 * @param heap heap
 * @param object object to be inserted
 * @return
 * * ::YADSL_HEAP_RET_OK, and object is inserted
 * * ::YADSL_HEAP_RET_FULL
*/
yadsl_HeapRet
yadsl_heap_insert(
	yadsl_HeapHandle* heap,
	yadsl_HeapObj* object);

/**
 * @brief Extract object from heap
 * @param heap heap
 * @param object_ptr extracted object
 * @return
 * * ::YADSL_HEAP_RET_OK, and *object_ptr is updated
 * * ::YADSL_HEAP_RET_EMPTY
*/
yadsl_HeapRet
yadsl_heap_extract(
	yadsl_HeapHandle* heap,
	yadsl_HeapObj** object_ptr);

/**
 * @brief Get heap maximum capacity
 * @param heap heap
 * @param size_ptr heap maximum capacity
 * @return
 * * ::YADSL_HEAP_RET_OK
*/
yadsl_HeapRet
yadsl_heap_size_get(
	yadsl_HeapHandle* heap,
	size_t* size_ptr);

/**
 * @brief Adjust heap maximum capacity
 * @param heap heap
 * @param new_size heap new maximum capacity
 * @return
 * * ::YADSL_HEAP_RET_OK, and *new_size is updated
 * * ::YADSL_HEAP_RET_SHRINK
 * * ::YADSL_HEAP_RET_MEMORY
*/
yadsl_HeapRet
yadsl_heap_resize(
	yadsl_HeapHandle* heap,
	size_t new_size);

/**
 * @brief Destroy heap and its remaining objects
 * @param heap heap
*/
void
yadsl_heap_destroy(
	yadsl_HeapHandle* heap);

/** @} */

#endif
