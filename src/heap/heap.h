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
//  A Heap starts empty and can be populated with arbitrary data
// ordered by an arbitrary comparison function. Items can be then
// inserted in or extracted from the heap at any given time.
//
// HINTS
// -----
//
//  The comparison function must be consistent for any two arbitrary
// data objects inserted. It will dictate which items must have
// higher priority over the others.
//
//  The return of a comparison function should be a boolean value which
// indicates whether the first item should be higher in the heap than
// the second item provided.
//
//  The Heap data structure acquires ownership of an object once it
// is successfully inserted and looses it if eventually extracted.
// When destructed, the heap calls an arbitrary free function, given
// by the caller at time of construction, for every remaining object.
//

typedef enum
{
	HEAP_OK = 0,
	HEAP_EMPTY,
	HEAP_FULL,
	HEAP_SHRINK,
	HEAP_MEMORY,
} HeapRet;

typedef struct Heap Heap;

#include <stddef.h>

//  ============= ========================================= 
//   heapCreate             Create an empty heap            
//  ============= ========================================= 
//   ppHeap        (ret) pointer to heap                    
//   initialSize   initial heap size (> 0)                  
//   cmpObjects    (opt) object comparison function         
//   freeObject    (opt) object deallocation function       
//   arg           additional argument given to cmpObjects  
//  ============= ========================================= 
//  [!] HEAP_MEMORY: try a smaller size

HeapRet heapCreate(Heap **ppHeap, size_t initialSize,
	int (*cmpObjects)(void *obj1, void *obj2, void *arg),
	void (*freeObject)(void *obj), void * arg);

//  ============ ======================= 
//   heapInsert   Insert object in heap  
//  ============ ======================= 
//   pHeap        pointer to heap        
//   object       object to be inserted  
//  ============ ======================= 
//  [!] HEAP_FULL: try resizing the heap

HeapRet heapInsert(Heap *pHeap, void *object);

//  ============= ======================== 
//   heapExtract   Insert object in heap   
//  ============= ======================== 
//   pHeap         pointer to heap         
//   pObject       (ret) extracted object  
//  ============= ======================== 
//  [!] HEAP_EMPTY

HeapRet heapExtract(Heap *pHeap, void **pObj);

//  ============= =========================== 
//   heapGetSize   Get heap maximum capacity  
//  ============= =========================== 
//   pHeap         pointer to heap            
//   pSize         (ret) heap size            
//  ============= =========================== 

HeapRet heapGetSize(Heap *pHeap, size_t *pSize);

//  ============ ============================== 
//   heapResize   Resize heap maximum capacity  
//  ============ ============================== 
//   pHeap        pointer to heap               
//   newSize      new heap size                 
//  ============ ============================== 
//  [!] HEAP_SHRINK: new size is smaller than the number of objects
//  [!] HEAP_MEMORY: try a smaller size

HeapRet heapResize(Heap *pHeap, size_t newSize);

//  ============= ======================================== 
//   heapDestroy   Destroy heap and its remaining objects  
//  ============= ======================================== 
//   pHeap         pointer to heap                         
//  ============= ======================================== 

void heapDestroy(Heap *pHeap);

#endif
