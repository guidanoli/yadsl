#include <list/list.h>

#include <stdlib.h>
#include <string.h>

#include <memdb/memdb.h>

#if defined(_MSC_VER)
# pragma warning(disable : 4550)
#endif

typedef struct
{
	size_t allocated; /**< Allocated number of objects */
	size_t size; /**< Current number of objects stored */
	yadsl_ListObj** objects; /**< Objects array */
}
yadsl_List;

#define cast_(list) yadsl_List* list ## _ = (yadsl_List*) list
#define default_(func) do { if (!func) func = func ## _default; } while(0)

static bool
equal_objs_func_default(
	yadsl_ListObj* obj1,
	yadsl_ListObj* obj2)
{
	return obj1 == obj2;
}

static void
free_obj_func_default(
	yadsl_ListObj* obj)
{}

static bool
copy_obj_func_default(
	yadsl_ListObj* obj,
	yadsl_ListObj** copy_ptr)
{
	*copy_ptr = obj;
	return true;
}

static yadsl_ListRet
yadsl_list_resize(
		yadsl_List* list,
		size_t new_size)
{
	yadsl_ListObj** objects;
	size_t new_allocated;

	/* k = (size * (9/8) + 6)
	   new_allocated = k - k % 4 */
	new_allocated = (new_size + (new_size >> 3) + 6) & ~(size_t)3;
	if (list->objects) {
		/* resize list */
		objects = realloc(list->objects, new_allocated * sizeof(*objects));
	} else {
		/* new list */
		objects = malloc(new_allocated * sizeof(*objects));
	}
	if (objects == NULL) {
		if (new_allocated > new_size)
			/* Needed a larger list, but could not (re)allocate */
			return YADSL_LIST_RET_MEMORY;
		else
			/* Could shrink list, but stayed with same size */
			return YADSL_LIST_RET_OK;
	}
	list->objects = objects;
	list->allocated = new_allocated;
	return YADSL_LIST_RET_OK;
}

yadsl_ListHandle*
yadsl_list_create()
{
	yadsl_List* list = malloc(sizeof *list);
	if (list) {
		list->size = 0;
		list->allocated = 0;
		list->objects = NULL;
	}
	return list;
}

yadsl_ListRet
yadsl_list_append(
		yadsl_ListHandle* list,
		yadsl_ListObj* obj)
{
	cast_(list);
	yadsl_ListRet ret;
	
	if (list_->size == list_->allocated)
		if (ret = yadsl_list_resize(list_, list_->size + 1))
			return ret;
	
	list_->objects[list_->size] = obj;
	++list_->size;
	
	return YADSL_LIST_RET_OK;
}

yadsl_ListRet
yadsl_list_insert(
		yadsl_ListHandle* list,
		intptr_t index,
		yadsl_ListObj* obj)
{
	cast_(list);
	yadsl_ListRet ret;
	size_t uindex, j;
	
	if (list_->size == list_->allocated)
		if (ret = yadsl_list_resize(list_, list_->size + 1))
			return ret;

	if (index < 0)
		index += list_->size + 1;
	
	/* Crop index to valid range */
	if (index < 0) {
		uindex = 0;
	} else if ((size_t) index > list_->size) {
		uindex = list_->size;
	} else {
		uindex = (size_t) index;
	}

	/* Shift objects to the right */
	j = list_->size;
	for (size_t i = 0; i < list_->size - uindex; ++i, --j)
		list_->objects[j] = list_->objects[j - 1];

	list_->objects[uindex] = obj;
	++list_->size;

	return YADSL_LIST_RET_OK;
}

yadsl_ListRet
yadsl_list_remove(
		yadsl_ListHandle* list,
		yadsl_ListObj* obj,
		yadsl_ListEqualObjsFunc equal_objs_func,
		yadsl_ListFreeObjFunc free_obj_func)
{
	cast_(list);
	default_(equal_objs_func);
	default_(free_obj_func);

	for (size_t i = 0; i < list_->size; ++i)
		if (equal_objs_func(list_->objects[i], obj)) {

			/* Free object */
			free_obj_func(list_->objects[i]);

			/* Shift objects to the left */
			for (size_t j = i+1; j < list_->size; ++j)
				list_->objects[j-1] = list_->objects[j];

			/* Shrink list size (never fails) */
			yadsl_list_resize(list_, list_->size - 1);

			--list_->size;

			return YADSL_LIST_RET_OK;
		}
	return YADSL_LIST_RET_NOT_FOUND;
}

yadsl_ListRet
yadsl_list_pop(
		yadsl_ListHandle* list,
		intptr_t index,
		yadsl_ListObj** obj_ptr)
{
	cast_(list);
	size_t uindex;

	if (index < 0)
		index += list_->size;

	if (index < 0 || (size_t) index >= list_->size)
		return YADSL_LIST_RET_INDEX;

	uindex = (size_t) index;

	*obj_ptr = list_->objects[uindex];

	/* Shift objects to the left */
	for (size_t i = uindex + 1; i < list_->size; ++i)
		list_->objects[i - 1] = list_->objects[i];

	/* Shrink list size (never fails) */
	yadsl_list_resize(list_, list_->size - 1);

	--list_->size;

	return YADSL_LIST_RET_OK;
}

void
yadsl_list_clear(
		yadsl_ListHandle* list,
		yadsl_ListFreeObjFunc free_obj_func)
{
	cast_(list);
	default_(free_obj_func);
	for (size_t i = 0; i < list_->size; ++i)
		free_obj_func(list_->objects[i]);
	if (list_->objects) {
		free(list_->objects);
		list_->objects = NULL;
	}
	list_->allocated = 0;
	list_->size = 0;
}

yadsl_ListHandle*
yadsl_list_copy(
		yadsl_ListHandle* list,
		yadsl_ListCopyObjFunc copy_obj_func,
		yadsl_ListFreeObjFunc free_obj_func)
{
	yadsl_ListHandle* newlist = yadsl_list_create();
	if (newlist) {
		cast_(list);
		cast_(newlist);
		default_(copy_obj_func);
		default_(free_obj_func);
		yadsl_ListObj** objects;
		newlist_->allocated = list_->allocated;
		objects = malloc(sizeof(*objects) * list_->allocated);
		if (!objects)
			goto fail;
		newlist_->objects = objects;
		for (size_t i = 0; i < list_->size; ++i, ++newlist_->size)
			if (!copy_obj_func(list_->objects[i], &newlist_->objects[i]))
				goto fail;
	}
	return newlist;
fail:
	yadsl_list_destroy(newlist, free_obj_func);
	return NULL;
}

size_t
yadsl_list_count(
		yadsl_ListHandle* list,
		yadsl_ListObj* obj,
		yadsl_ListEqualObjsFunc equal_objs_func)
{
	size_t count = 0;
	cast_(list);
	default_(equal_objs_func);
	for (size_t i = 0; i < list_->size; ++i)
		if (equal_objs_func(list_->objects[i], obj))
			++count;
	return count;
}

yadsl_ListRet
yadsl_list_index(
		yadsl_ListHandle* list,
		yadsl_ListObj* obj,
		yadsl_ListEqualObjsFunc equal_objs_func,
		size_t* index_ptr)
{
	cast_(list);
	default_(equal_objs_func);
	for (size_t i = 0; i < list_->size; ++i)
		if (equal_objs_func(list_->objects[i], obj)) {
			*index_ptr = i;
			return YADSL_LIST_RET_OK;
		}
	return YADSL_LIST_RET_NOT_FOUND;

}

size_t
yadsl_list_size(
		yadsl_ListHandle* list)
{
	cast_(list);
	return list_->size;
}

void
yadsl_list_iter(
	yadsl_ListHandle* list,
	yadsl_ListIterFunc iter_func)
{
	cast_(list);
	for (size_t i = 0; i < list_->size; ++i)
		iter_func(list_->objects[i]);
}

yadsl_ListRet
yadsl_list_at(
	yadsl_ListHandle* list,
	intptr_t index,
	yadsl_ListObj** obj_ptr)
{
	cast_(list);
	size_t uindex;

	if (index < 0)
		index += list_->size;

	if (index < 0 || (size_t) index >= list_->size)
		return YADSL_LIST_RET_INDEX;

	uindex = (size_t) index;

	*obj_ptr = list_->objects[uindex];
	return YADSL_LIST_RET_OK;
}

void
yadsl_list_destroy(
		yadsl_ListHandle* list,
		yadsl_ListFreeObjFunc free_obj_func)
{
	cast_(list);
	default_(free_obj_func);
	for (size_t i = 0; i < list_->size; ++i)
		free_obj_func(list_->objects[i]);
	if (list_->objects)
		free(list_->objects);
	free(list_);
}

