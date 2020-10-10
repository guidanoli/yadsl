#ifndef __YADSL_LIST_H__
#define __YADSL_LIST_H__

/**
 * \defgroup list List
 * @brief Mutable sequence of elements
 *
 * Negative indexation is allowed and has the
 * same semantics as in Python lists.
 *
 * @{
*/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Return condition of List functions
*/
typedef enum
{
	YADSL_LIST_RET_OK = 0, /**< All went ok */
	YADSL_LIST_RET_MEMORY, /**< Could not allocate memory */
	YADSL_LIST_RET_INDEX, /**< Invalid index */
	YADSL_LIST_RET_NOT_FOUND, /**< Object not found */
}
yadsl_ListRet;

typedef void yadsl_ListHandle; /**< List handle */
typedef void yadsl_ListObj; /**< List object */

/**
 * @brief List object freeing function
 * @param obj object
*/
typedef void
(yadsl_ListFreeObjFunc)(
		yadsl_ListObj* obj);

/**
 * @brief List object equality function
 * @param obj1 object 1
 * @param obj2 object 2
 * @return whether objects are equal or not
 */
typedef bool
(yadsl_ListEqualObjsFunc)(
		yadsl_ListObj *obj1,
		yadsl_ListObj *obj2);

/**
 * @brief List object copy function
 * @param obj object
 * @return copied object
*/
typedef yadsl_ListObj*
(yadsl_ListCopyObjFunc)(
		yadsl_ListObj *obj);

/**
 * @brief Create an empty list
 * @return newly created list or NULL if could not allocate memory
*/
yadsl_ListHandle*
yadsl_list_create();

/**
 * @brief Appends object to the end of the list
 * @param list list
 * @param obj object to be appended
 * @return
 * * ::YADSL_LIST_RET_OK, and object is appended
 * * ::YADSL_LIST_RET_MEMORY
*/
yadsl_ListRet
yadsl_list_append(
		yadsl_ListHandle* list,
		yadsl_ListObj* obj);

/**
 * @brief Inserts object in a specific index
 * @param list list
 * @param index in which value is inserted
 * @param obj object to be inserted
 * @return
 * * ::YADSL_LIST_RET_OK, and object is inserted
 * * ::YADSL_LIST_RET_MEMORY
 * * ::YADSL_LIST_RET_INDEX
*/
yadsl_ListRet
yadsl_list_insert(
		yadsl_ListHandle* list,
		intptr_t index,
		yadsl_ListObj* obj);

/**
 * @brief Remove object from list
 * @param list list
 * @param obj object to be removed
 * @param equal_objs_func function that checks if two
 *                        object are equal or not
 * @return
 * * ::YADSL_LIST_RET_OK, and object is removed
 * * ::YADSL_LIST_RET_NOT_FOUND
*/
yadsl_ListRet
yadsl_list_remove(
		yadsl_ListHandle* list,
		yadsl_ListObj* obj,
		yadsl_ListEqualObjsFunc equal_objs_func);

/**
 * @brief Remove object from list and return it
 * @param list list
 * @param index index in which value will be removed
 * @param obj_ptr removed object
 * @return
 * * ::YADSL_LIST_RET_OK, and *obj_ptr is updated
 * * ::YADSL_LIST_RET_INDEX
 */
yadsl_ListRet
yadsl_list_pop(
		yadsl_ListHandle* list,
		intptr_t index,
		yadsl_ListObj** obj_ptr);

/**
 * @brief Clear list
 * @param list
 * @param free_obj_func function that frees object
*/
void
yadsl_list_clear(
		yadsl_ListHandle* list,
		yadsl_ListFreeObjFunc* free_obj_func);

/**
 * @brief Copy list
 * @param list
 * @param copy_obj_func function that copies each object
 * @return newly copied list or NULL on failure
*/
yadsl_ListHandle*
yadsl_list_copy(
		yadsl_ListHandle* list,
		yadsl_ListCopyObjFunc copy_obj_func);

/**
 * @brief Counts incidences of object in list
 * @param list
 * @param obj object to be counted
 * @param equal_objs_func function that tests if two
 *                        objects are equal or not
 * @return number of incidences
*/
size_t
yadsl_list_count(
		yadsl_ListHandle* list,
		yadsl_ListObj* obj,
		yadsl_ListEqualObjsFunc equal_objs_func);

/**
 * @brief Get index of object in list
 * @param list
 * @param obj object to be found
 * @param equal_objs_func function that tests if two
 *                        objects are equal or not
 * @param index_ptr index
 * @return
 * * ::YADSL_LIST_RET_OK, and *index_ptr is updated
 * * ::YADSL_LIST_RET_NOT_FOUND
 */
yadsl_ListRet
yadsl_list_index(
		yadsl_ListHandle* list,
		yadsl_ListObj* obj,
		yadsl_ListEqualObjsFunc equal_objs_func,
		size_t* index_ptr);

/**
 * @brief Get number of elements in list
 * @param list list
 * @return list size
*/
size_t
yadsl_list_size(
		yadsl_ListHandle* list);

/**
 * @brief Destroy a list
 * @param list list to be destroyed
 * @param free_obj_func function that frees object
*/
void
yadsl_list_destroy(
		yadsl_ListHandle* list,
		yadsl_ListFreeObjFunc* free_obj_func);

/** @} */

#endif
