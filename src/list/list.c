#include <list/list.h>

#include <stdlib.h>

yadsl_ListHandle*
yadsl_list_create()
{
	return NULL;
}

yadsl_ListRet
yadsl_list_append(
		yadsl_ListHandle* list,
		yadsl_ListObj* obj)
{
	return YADSL_LIST_RET_OK;
}

yadsl_ListRet
yadsl_list_insert(
		yadsl_ListHandle* list,
		intptr_t index,
		yadsl_ListObj* obj)
{
	return YADSL_LIST_RET_OK;
}

yadsl_ListRet
yadsl_list_remove(
		yadsl_ListHandle* list,
		yadsl_ListObj* obj,
		yadsl_ListEqualObjsFunc equal_objs_func)
{
	return YADSL_LIST_RET_OK;
}

yadsl_ListRet
yadsl_list_pop(
		yadsl_ListHandle* list,
		intptr_t index,
		yadsl_ListObj** obj_ptr)
{
	return YADSL_LIST_RET_OK;
}

void
yadsl_list_clear(
		yadsl_ListHandle* list,
		yadsl_ListFreeObjFunc* free_obj_func){}

yadsl_ListHandle*
yadsl_list_copy(
		yadsl_ListHandle* list,
		yadsl_ListCopyObjFunc copy_obj_func)
{
	return NULL;
}

size_t
yadsl_list_count(
		yadsl_ListHandle* list,
		yadsl_ListObj* obj,
		yadsl_ListEqualObjsFunc equal_objs_func)
{
	return 0;
}

yadsl_ListRet
yadsl_list_index(
		yadsl_ListHandle* list,
		yadsl_ListObj* obj,
		yadsl_ListEqualObjsFunc equal_objs_func,
		size_t* index_ptr)
{
	return YADSL_LIST_RET_OK;
}

size_t
yadsl_list_size(
		yadsl_ListHandle* list)
{
	return 0;
}

void
yadsl_list_destroy(
		yadsl_ListHandle* list,
		yadsl_ListFreeObjFunc* free_obj_func){}

