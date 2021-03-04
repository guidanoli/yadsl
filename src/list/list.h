#ifndef __YADSL_LIST_H__
#define __YADSL_LIST_H__

/**
 * \defgroup list List
 * @brief Mutable sequence of elements
 *
 * Negative indices
 * ----------------
 * 
 * Negative indexation is allowed and has the
 * same semantics as in Python lists. For example:
 * 
 * * -1 represents the last item in the list
 * * -2 represents the penultimate item in the list
 * 
 * Some callbacks are nullable. When NULL,
 * these callbacks are assigned to default ones.
 * 
 * Default callbacks
 * -----------------
 * 
 * * yadsl_ListFreeObjFunc -> does nothing
 * * yadsl_ListEqualObjsFunc -> shallow comparison
 * * yadsl_ListCopyObjFunc -> shallow copy
 *
 * Complexity
 * ----------
 * 
 * | Function | Time | Space |
 * | :-: | :-: | :-: |
 * | ::yadsl_list_create  | O(1) | O(1) |
 * | ::yadsl_list_append  | O(1) | O(log n) |
 * | ::yadsl_list_insert  | O(n) | O(log n) |
 * | ::yadsl_list_remove  | O(n) | O(1) |
 * | ::yadsl_list_pop     | O(n) | O(1) |
 * | ::yadsl_list_clear   | O(n) | O(1) |
 * | ::yadsl_list_copy    | O(n) | O(n) |
 * | ::yadsl_list_count   | O(n) | O(1) |
 * | ::yadsl_list_index   | O(n) | O(1) |
 * | ::yadsl_list_size    | O(1) | O(1) |
 * | ::yadsl_list_iter    | O(n) | O(1) |
 * | ::yadsl_list_at      | O(1) | O(1) |
 * | ::yadsl_list_destroy | O(n) | O(1) |
 * 
 * Some observations:
 * 
 * * Popping the last element (index = -1) has a time complexity of O(1).
 * * Inserting an element at the end of a list (index = -1) is analogous to appending it.
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
 * @param copy_ptr copied object
 * @return success (copy_ptr is updated)
*/
typedef bool
(yadsl_ListCopyObjFunc)(
		yadsl_ListObj *obj,
		yadsl_ListObj **copy_ptr);

/**
 * @brief List iteration function
 * @param obj current object
*/
typedef void
(yadsl_ListIterFunc) (
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
 * @param index index in which value is inserted
 *              if it surpasses the range, it crops to the nearest
 *              valid value (e.g. on a list with two elements, 10 goes to 2)
 * @param obj object to be inserted
 * @return
 * * ::YADSL_LIST_RET_OK, and object is inserted
 * * ::YADSL_LIST_RET_MEMORY
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
 *                        object are equal or not (nullable)
 * @param free_obj_func function that frees object (nullable)
 * @return
 * * ::YADSL_LIST_RET_OK, and object is removed
 * * ::YADSL_LIST_RET_NOT_FOUND
*/
yadsl_ListRet
yadsl_list_remove(
		yadsl_ListHandle* list,
		yadsl_ListObj* obj,
		yadsl_ListEqualObjsFunc equal_objs_func,
		yadsl_ListFreeObjFunc free_obj_func);

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
 * @param free_obj_func function that frees object (nullable)
*/
void
yadsl_list_clear(
		yadsl_ListHandle* list,
		yadsl_ListFreeObjFunc free_obj_func);

/**
 * @brief Copy list
 * @param list
 * @param copy_obj_func function that copies each object (nullable)
 * @param free_obj_func function that deallocates object (nullable)
 * @return newly copied list or NULL on failure
*/
yadsl_ListHandle*
yadsl_list_copy(
		yadsl_ListHandle* list,
		yadsl_ListCopyObjFunc copy_obj_func,
		yadsl_ListFreeObjFunc free_obj_func);

/**
 * @brief Counts incidences of object in list
 * @param list
 * @param obj object to be counted
 * @param equal_objs_func function that tests if two
 *                        objects are equal or not (nullable)
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
 * @brief Iterate throught list
 * @param list list
 * @param iter_func iteration function
*/
void
yadsl_list_iter(
		yadsl_ListHandle* list,
		yadsl_ListIterFunc iter_func);

/**
 * @brief Obtain object at a given index
 * @param list list
 * @param index index
 * @param obj_ptr obtained object
 * @return
 * * ::YADSL_LIST_RET_OK, and *obj_ptr is updated
 * * ::YADSL_LIST_RET_INDEX
*/
yadsl_ListRet
yadsl_list_at(
		yadsl_ListHandle* list,
		intptr_t index,
		yadsl_ListObj** obj_ptr);

/**
 * @brief Destroy a list
 * @param list list to be destroyed
 * @param free_obj_func function that frees object (nullable)
*/
void
yadsl_list_destroy(
		yadsl_ListHandle* list,
		yadsl_ListFreeObjFunc free_obj_func);

/** @} */

#endif
