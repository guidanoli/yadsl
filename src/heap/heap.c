#include <heap/heap.h>

#include <stdlib.h>
#include <stdint.h>

#include <memdb/memdb.h>

typedef struct
{
	yadsl_HeapObj** arr; /**< Array representation of binary tree */
	size_t size; /**< Array total size */
	size_t last; /**< Index of rightmost empty position */
	yadsl_HeapObjCmpFunc cmp_objs_func;
	yadsl_HeapObjFreeFunc free_obj_func;
	yadsl_HeapObjCmpArg* cmp_objs_arg;
}
yadsl_Heap;

#define YADSL_HEAP_ROOT 0
#define YADSL_HEAP_IS_ROOT(x) (x == YADSL_HEAP_ROOT)
#define YADSL_HEAP_EXISTS(x, heap) (x < heap->last)
#define YADSL_HEAP_LEFT(x) ((x << 1) + 1)
#define YADSL_HEAP_RIGHT(x) ((x + 1) << 1)
#define YADSL_HEAP_PARENT(x) ((x - 1) >> 1)
#define YADSL_HEAP_HAS_CHILD(x, heap) (x <= YADSL_HEAP_PARENT(heap->last - 1))

static void
yadsl_heap_swap_internal(
	yadsl_Heap* heap,
	size_t* a,
	size_t* b);

static int
yadsl_heap_default_cmp_objs_func_internal(
	yadsl_HeapObj* obj1,
	yadsl_HeapObj* obj2,
	yadsl_HeapObjCmpArg* arg);

yadsl_HeapHandle*
yadsl_heap_create(
	size_t initial_size,
	yadsl_HeapObjCmpFunc cmp_objs_func,
	yadsl_HeapObjFreeFunc free_obj_func,
	yadsl_HeapObjCmpArg* cmp_objs_arg)
{
	yadsl_Heap* heap;

	if (initial_size == 0 ||
		initial_size > SIZE_MAX / sizeof(yadsl_HeapObj*))
		return NULL;

	heap = malloc(sizeof(*heap));
	if (!heap)
		goto fail1;

	heap->arr = malloc(sizeof(*heap->arr) * initial_size);
	if (!heap->arr)
		goto fail2;

	heap->last = 0;
	if (cmp_objs_func) {
		heap->cmp_objs_func = cmp_objs_func;
		heap->cmp_objs_arg = cmp_objs_arg;
	} else {
		heap->cmp_objs_func = yadsl_heap_default_cmp_objs_func_internal;
	}
	heap->free_obj_func = free_obj_func;
	heap->size = initial_size;

	return heap;
fail2:
	free(heap);
fail1:
	return NULL;
}

yadsl_HeapRet
yadsl_heap_insert(
	yadsl_HeapHandle* heap,
	yadsl_HeapObj* object)
{
	void* object_parent;
	size_t index, parent_index;
	yadsl_Heap* heap_ = (yadsl_Heap*) heap;

	if (heap_->last == heap_->size)
		return YADSL_HEAP_RET_FULL;

	index = heap_->last;
	heap_->arr[index] = object;
	++heap_->last;

	while (!YADSL_HEAP_IS_ROOT(index)) {
		parent_index = YADSL_HEAP_PARENT(index);
		object_parent = heap_->arr[parent_index];
		if (heap_->cmp_objs_func(object, object_parent, heap_->cmp_objs_arg))
			yadsl_heap_swap_internal(heap_, &index, &parent_index);
		else
			break;
	}

	return YADSL_HEAP_RET_OK;
}

yadsl_HeapRet
yadsl_heap_extract(
	yadsl_HeapHandle* heap,
	yadsl_HeapObj** object_ptr)
{
	size_t index = YADSL_HEAP_ROOT;
	yadsl_HeapObj* object;
	yadsl_Heap* heap_ = (yadsl_Heap*) heap;

	if (heap_->last == YADSL_HEAP_ROOT)
		return YADSL_HEAP_RET_EMPTY;

	*object_ptr = heap_->arr[YADSL_HEAP_ROOT];
	object = heap_->arr[--heap_->last];
	heap_->arr[YADSL_HEAP_ROOT] = object;

	while (YADSL_HEAP_EXISTS(index, heap_) && YADSL_HEAP_HAS_CHILD(index, heap_)) {
		size_t left_index = YADSL_HEAP_LEFT(index);
		size_t right_index = YADSL_HEAP_RIGHT(index);
		yadsl_HeapObj* left_object, * right_object;

		if (!YADSL_HEAP_EXISTS(right_index, heap_)) {
			if (heap_->cmp_objs_func(heap_->arr[left_index], object, heap_->cmp_objs_arg))
				yadsl_heap_swap_internal(heap_, &index, &left_index);
			break;
		}

		left_object = heap_->arr[left_index];
		right_object = heap_->arr[right_index];

		if (heap_->cmp_objs_func(object, left_object, heap_->cmp_objs_arg) &&
			heap_->cmp_objs_func(object, right_object, heap_->cmp_objs_arg))
			break;
		if (heap_->cmp_objs_func(left_object, right_object, heap_->cmp_objs_arg))
			yadsl_heap_swap_internal(heap_, &left_index, &index);
		else
			yadsl_heap_swap_internal(heap_, &right_index, &index);
	}

	return YADSL_HEAP_RET_OK;
}

yadsl_HeapRet
yadsl_heap_size_get(
	yadsl_HeapHandle* heap,
	size_t* size_ptr)
{
	*size_ptr = ((yadsl_Heap*) heap)->size;
	return YADSL_HEAP_RET_OK;
}

yadsl_HeapRet
yadsl_heap_resize(
	yadsl_HeapHandle* heap,
	size_t new_size)
{
	void* new_arr;
	yadsl_Heap* heap_ = (yadsl_Heap*) heap;

	if (new_size == 0)
		return YADSL_HEAP_RET_MEMORY;
	if (new_size < heap_->last)
		return YADSL_HEAP_RET_SHRINK;
	if (new_size == heap_->size)
		return YADSL_HEAP_RET_OK;
	if (new_size > SIZE_MAX / sizeof(void*))
		return YADSL_HEAP_RET_MEMORY;

	new_arr = realloc(heap_->arr, sizeof(void*) * new_size);
	if (!new_arr)
		return YADSL_HEAP_RET_MEMORY;

	heap_->arr = new_arr;
	heap_->size = new_size;

	return YADSL_HEAP_RET_OK;
}

void
yadsl_heap_destroy(
	yadsl_HeapHandle* heap)
{
	yadsl_Heap* heap_ = (yadsl_Heap*) heap;

	if (heap_ == NULL)
		return;

	if (heap_->free_obj_func)
		while (heap_->last--)
			heap_->free_obj_func(heap_->arr[heap_->last]);

	free(heap_->arr);
	free(heap_);
}

// Swaps the objects at indexes a and b
// and the indexes passed by refference too
// Makes no safety checks whatsoever
void
yadsl_heap_swap_internal(
	yadsl_Heap* heap,
	size_t* a,
	size_t* b)
{
	void* objA = heap->arr[*a], * objB = heap->arr[*b];
	size_t temp;
	heap->arr[*a] = objB;
	heap->arr[*b] = objA;
	temp = *a;
	*a = *b;
	*b = temp;
}

int
yadsl_heap_default_cmp_objs_func_internal(
	yadsl_HeapObj* obj1,
	yadsl_HeapObj* obj2,
	yadsl_HeapObjCmpArg* arg)
{
	return obj1 < obj2;
}