#include <heap/heap.h>

#include <stdlib.h>
#include <stdint.h>

#include <memdb/memdb.h>

struct Heap
{
	void **arr; /* Array representation of binary tree */
	size_t size; /* Array total size */
	size_t last; /* Index of rightmost empty position */
	int (*cmpObjs)(void *obj1, void *obj2, void *arg);
	void (*freeObj)(void *obj);
	void *arg;
};

#define _ROOT 0
#define _ISROOT(x) (x == _ROOT)
#define _EXISTS(x, pHeap) (x < pHeap->last)
#define _LEFT(x) ((x << 1) + 1)
#define _RIGHT(x) ((x + 1) << 1)
#define _PARENT(x) ((x - 1) >> 1)
#define _HAS_CHILD(x, pHeap) (x <= _PARENT(pHeap->last - 1))

void _swap(Heap *pHeap, size_t *a, size_t *b);

int _defaultCmp(void *obj1, void *obj2, void *arg)
{
	return obj1 < obj2;
}

HeapRet heapCreate(Heap **ppHeap, size_t initialSize,
	int (*cmpObjs)(void *obj1, void *obj2, void *arg),
	void (*freeObj)(void *obj), void *arg)
{
	Heap *pHeap;
	void **arr;
	if (initialSize == 0)
		return HEAP_MEMORY;
	if (initialSize > SIZE_MAX / sizeof(void *))
		return HEAP_MEMORY;
	pHeap = malloc(sizeof(struct Heap));
	arr = malloc(sizeof(void *) * initialSize);
	if (pHeap == NULL || arr == NULL) {
		if (pHeap) free(pHeap);
		if (arr) free(arr);
		return HEAP_MEMORY;
	}
	pHeap->last = 0;
	pHeap->arr = arr;
	pHeap->cmpObjs = cmpObjs ? cmpObjs : _defaultCmp;
	pHeap->freeObj = freeObj;
	pHeap->size = initialSize;
	pHeap->arg = cmpObjs ? arg : NULL;
	*ppHeap = pHeap;
	return HEAP_OK;
}

HeapRet heapInsert(Heap *pHeap, void *obj)
{
	void *parentObj;
	size_t newi, parenti;
	if (pHeap->last == pHeap->size)
		return HEAP_FULL;
	newi = pHeap->last;
	pHeap->arr[newi] = obj;
	++pHeap->last;
	while (!_ISROOT(newi)) {
		parenti = _PARENT(newi);
		parentObj = pHeap->arr[parenti];
		if (pHeap->cmpObjs(obj, parentObj, pHeap->arg))
			_swap(pHeap, &newi, &parenti);
		else
			break;
	}
	return HEAP_OK;
}

HeapRet heapExtract(Heap *pHeap, void **pObj)
{
	size_t obji = _ROOT;
	void *obj;
	if (pHeap->last == _ROOT)
		return HEAP_EMPTY;
	*pObj = pHeap->arr[_ROOT];
	obj = pHeap->arr[--pHeap->last];
	pHeap->arr[_ROOT] = obj;
	while (_EXISTS(obji, pHeap) && _HAS_CHILD(obji, pHeap)) {
		size_t lefti = _LEFT(obji), righti = _RIGHT(obji);
		void *leftobj, *rightobj;
		if (!_EXISTS(righti, pHeap)) {
			if (pHeap->cmpObjs(pHeap->arr[lefti], obj, pHeap->arg))
				_swap(pHeap, &obji, &lefti);
			break;
		}
		leftobj = pHeap->arr[lefti];
		rightobj = pHeap->arr[righti];
		if (pHeap->cmpObjs(obj, leftobj, pHeap->arg) &&
			pHeap->cmpObjs(obj, rightobj, pHeap->arg))
			break;
		if (pHeap->cmpObjs(leftobj, rightobj, pHeap->arg))
			_swap(pHeap, &lefti, &obji);
		else
			_swap(pHeap, &righti, &obji);
	}
	return HEAP_OK;
}

HeapRet heapGetSize(Heap *pHeap, size_t *pSize)
{
	*pSize = pHeap->size;
	return HEAP_OK;
}

HeapRet heapResize(Heap *pHeap, size_t newSize)
{
	void *new_arr;
	if (newSize == 0)
		return HEAP_MEMORY;
	if (newSize < pHeap->last)
		return HEAP_SHRINK;
	if (newSize == pHeap->size)
		return HEAP_OK;
	if (newSize > SIZE_MAX / sizeof(void *))
		return HEAP_MEMORY;
	new_arr = realloc(pHeap->arr, sizeof(void *) * newSize);
	if (new_arr == NULL)
		return HEAP_MEMORY;
	pHeap->arr = new_arr;
	pHeap->size = newSize;
	return HEAP_OK;
}

void heapDestroy(Heap *pHeap)
{
	if (pHeap == NULL)
		return;
	if (pHeap->freeObj)
		while (pHeap->last--)
			pHeap->freeObj(pHeap->arr[pHeap->last]);
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