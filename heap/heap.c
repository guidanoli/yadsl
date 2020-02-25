#include "heap.h"

#include <stdlib.h>
#include <stdint.h>

#ifdef _DEBUG
#include "memdb.h"
#endif

struct Heap
{
	void **arr; /* Array representation of binary tree */
	size_t size; /* Array total size */
	size_t last; /* Index of rightmost empty position */
	int (*cmpObjs)(void *obj1, void *obj2);
	void (*freeObj)(void *obj);
};

#define _ROOT 0
#define _ISROOT(x) (x == _ROOT)
#define _EXISTS(x, pHeap) (x < pHeap->last)
#define _LEFT(x) ((x << 1) + 1)
#define _RIGHT(x) ((x + 1) << 1)
#define _PARENT(x) ((x - 1) >> 1)
#define _HAS_CHILD(x, pHeap) (x <= _PARENT(pHeap->last - 1))

void _swap(Heap *pHeap, size_t *a, size_t *b);

HeapReturnID heapCreate(Heap **ppHeap,
	size_t initialSize,
	int (*cmpObjs)(void *obj1, void *obj2),
	void (*freeObj)(void *obj))
{
	Heap *pHeap;
	void **arr;
	if (ppHeap == NULL || !initialSize)
		return HEAP_RETURN_INVALID_PARAMETER;
	if (initialSize > SIZE_MAX / sizeof(void *))
		return HEAP_RETURN_MEMORY;
	pHeap = malloc(sizeof(struct Heap));
	arr = malloc(sizeof(void *) * initialSize);
	if (pHeap == NULL || arr == NULL) {
		if (pHeap) free(pHeap);
		if (arr) free(arr);
		return HEAP_RETURN_MEMORY;
	}
	pHeap->last = 0;
	pHeap->arr = arr;
	pHeap->cmpObjs = cmpObjs;
	pHeap->freeObj = freeObj;
	pHeap->size = initialSize;
	*ppHeap = pHeap;
	return HEAP_RETURN_OK;
}

HeapReturnID heapInsert(Heap *pHeap, void *obj)
{
	void *parentObj;
	size_t newi, parenti;
	if (pHeap == NULL)
		return HEAP_RETURN_INVALID_PARAMETER;
	if (pHeap->last == pHeap->size)
		return HEAP_RETURN_FULL;
	newi = pHeap->last;
	pHeap->arr[newi] = obj;
	++pHeap->last;
	while (!_ISROOT(newi)) {
		parenti = _PARENT(newi);
		parentObj = pHeap->arr[parenti];
		if (pHeap->cmpObjs(obj, parentObj))
			_swap(pHeap, &newi, &parenti);
		else
			break;
	}
	return HEAP_RETURN_OK;
}

HeapReturnID heapExtract(Heap *pHeap, void **pObj)
{
	size_t obji = _ROOT;
	void *obj;
	if (pHeap == NULL || pObj == NULL)
		return HEAP_RETURN_INVALID_PARAMETER;
	if (pHeap->last == _ROOT)
		return HEAP_RETURN_EMPTY;
	*pObj = pHeap->arr[_ROOT];
	obj = pHeap->arr[--pHeap->last];
	pHeap->arr[_ROOT] = obj;
	while (_EXISTS(obji, pHeap) && _HAS_CHILD(obji, pHeap)) {
		size_t lefti = _LEFT(obji), righti = _RIGHT(obji);
		void *leftobj, *rightobj;
		if (!_EXISTS(righti, pHeap)) {
			if (pHeap->cmpObjs(pHeap->arr[lefti], obj))
				_swap(pHeap, &obji, &lefti);
			break;
		}
		leftobj = pHeap->arr[lefti];
		rightobj = pHeap->arr[righti];
		if (pHeap->cmpObjs(obj, leftobj) && pHeap->cmpObjs(obj, rightobj))
			break;
		if (pHeap->cmpObjs(leftobj, rightobj))
			_swap(pHeap, &lefti, &obji);
		else
			_swap(pHeap, &righti, &obji);
	}
	return HEAP_RETURN_OK;
}

HeapReturnID heapResize(Heap *pHeap, size_t newSize)
{
	void *new_arr;
	if (pHeap == NULL || newSize == 0)
		return HEAP_RETURN_INVALID_PARAMETER;
	if (newSize < pHeap->last)
		return HEAP_RETURN_SHRINK;
	if (newSize == pHeap->size)
		return HEAP_RETURN_OK;
	if (newSize > SIZE_MAX / sizeof(void *))
		return HEAP_RETURN_MEMORY;
	new_arr = realloc(pHeap->arr, sizeof(void *) * newSize);
	if (new_arr == NULL)
		return HEAP_RETURN_MEMORY;
	pHeap->arr = new_arr;
	pHeap->size = newSize;
	return HEAP_RETURN_OK;
}

void heapDestroy(Heap *pHeap)
{
	if (pHeap == NULL)
		return;
	free(pHeap->arr);
	free(pHeap);
}

// Swaps the objects at indexes a and b
// and the indexes passed by refference too
// Makes no safety checks whatsoever
void _swap(Heap *pHeap, size_t *a, size_t *b)
{
	void *objA = pHeap->arr[*a], *objB = pHeap->arr[*b];
	size_t temp;
	pHeap->arr[*a] = objB;
	pHeap->arr[*b] = objA;
	temp = *a;
	*a = *b;
	*b = temp;
}