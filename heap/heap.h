#ifndef __HEAP_H__
#define __HEAP_H__

//  
//      __  __               
//     / / / /__  ____ _____ 
//    / /_/ / _ \/ __ `/ __ \
//   / __  /  __/ /_/ / /_/ /
//  /_/ /_/\___/\__,_/ .___/ 
//                  /_/      
//  
// A Heap starts empty and can be populated with arbitrary data
// ordered by an arbitrary comparison function. Items can be then
// inserted in or extracted from the heap at any given time.
//
// The comparison function must be consistent for any two arbitrary
// data objects inserted. It will dictate which items must have
// higher priority over the others.
// 
// The Heap data structure acquires ownership of an object once it
// is successfully inserted and looses it if eventually extracted.
// When destructed, the heap calls an arbitrary free function, given
// by the caller at time of construction, for every remaining object.
//
// HINT: HEAP_RETURN_OK will always be 0, so that you can easily
// assert that a function has succeeded or not.
//

typedef enum
{
	// SEMANTIC RETURN VALUES

	/* All went ok */
	HEAP_RETURN_OK = 0,

	/* Heap is empty */
	HEAP_RETURN_EMPTY,

	/* Heap is full */
	HEAP_RETURN_FULL,
	
	/* Cannot shrink */
	HEAP_RETURN_SHRINK,

	// ERROR RETURN VALUES

	/* An invalid parameter was passed */
	HEAP_RETURN_INVALID_PARAMETER,

	/* Could not allocate memory */
	HEAP_RETURN_MEMORY,

} HeapReturnID;

typedef struct Heap Heap;

#include <stddef.h>

/**
* Create an empty heap
* ppHeap       (return) pointer to heap
* initialSize  initial heap size
* cmpObjs      comparison function or NULL (shallow comparison)
*                Returns boolean value of whether
*                obj1 should be higher in the heap than obj2
* freeObj      deallocation function or NULL (no ownership)
*                Property deallocates object
* Possible errors:
* HEAP_RETURN_INVALID_PARAMETER
*	- "ppHeap" is NULL
*	- "initialSize" is 0
* HEAP_RETURN_MEMORY
*	- HINT: you may try a smaller initial size
*/
HeapReturnID heapCreate(Heap **ppHeap, size_t initialSize,
	int (*cmpObjs)(void *obj1, void *obj2),
	void (*freeObj)(void *obj));

/**
* Insert object in heap
* pHeap   pointer to heap
* obj     object to be inserted
* Possible errors:
* HEAP_RETURN_INVALID_PARAMETER
*	- "pHeap" is NULL
* HEAP_RETURN_FULL
*	- HINT: you may call heapResize
*/
HeapReturnID heapInsert(Heap *pHeap, void *obj);

/**
* Extract object from heap
* pHeap   pointer to heap
* pObj    (return) extracted object
* Possible errors:
* HEAP_RETURN_INVALID_PARAMETER
*	- "pHeap" is NULL
*	- "pObj" is NULL
* HEAP_RETURN_EMPTY
*/
HeapReturnID heapExtract(Heap *pHeap, void **pObj);

/**
* Resize heap maximum capacity
* pHeap    pointer to heap
* newSize  new size
* Possible errors:
* HEAP_RETURN_INVALID_PARAMETER
*	- "pHeap" is NULL
*	- "newSize" is 0
* HEAP_RETURN_SHRINK
*	- new size is smaller than the current size
* HEAP_RETURN_MEMORY
*	- HINT: you may try a smaller size
* [!] If the current size is equal to the new size
* HEAP_RETURN_OK will be returned
*/
HeapReturnID heapResize(Heap *pHeap, size_t newSize);

/**
* Destroy a heap and its objects, if a deallocation
* function was passed at the time of construction
* pHeap   pointer to heap
*/
void heapDestroy(Heap *pHeap);

#endif
