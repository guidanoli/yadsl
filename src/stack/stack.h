#ifndef __YADSL_STACK_H__
#define __YADSL_STACK_H__

/**
 * \defgroup stack Stack
 * @brief Generic stack
 * 
 * A Stack starts empty. You can add and
 * remove objects (if not empty), and check
 * whether the stack is empty or not.
 * Objects can assume the value NULL (0).
 * 
 * @{
*/

#include <stdbool.h>

/**
 * @brief Value returned by Stack functions
*/
typedef enum
{
	YADSL_STACK_RET_OK = 0, /**< All went ok */
	YADSL_STACK_RET_EMPTY, /**< Stack is empty */
	YADSL_STACK_RET_MEMORY, /**< Could not allocate memory */
}
yadsl_StackRet;

typedef void yadsl_StackHandle; /**< Stack handle */
typedef void yadsl_StackItemObj; /**< Stack item object */

/**
 * @brief Function responsible for freeing item
 * @param item item object
*/
typedef void
(*yadsl_StackItemFreeFunc)(
	yadsl_StackItemObj* item);

/**
 * @brief Create stack
 * @return newly created stack or NULL if could not allocate memory
*/
yadsl_StackHandle*
yadsl_stack_create();

/**
 * @brief Add object to stack
 * @param stack stack
 * @param object object to be added
 * @return
 * * ::YADSL_STACK_RET_OK, and object is added
 * * ::YADSL_STACK_RET_MEMORY
*/
yadsl_StackRet
yadsl_stack_item_add(
	yadsl_StackHandle *stack,
	yadsl_StackItemObj *object);

/**
 * @brief Check if stack is empty
 * @param stack stack
 * @param is_empty_ptr whether stack is empty or not
 * @return
 * * ::YADSL_STACK_RET_OK, and *is_empty_ptr is updated
*/
yadsl_StackRet
yadsl_stack_empty_check(
	yadsl_StackHandle *stack,
	bool *is_empty_ptr);

/**
 * @brief Remove object from stack
 * @param stack stack
 * @param object_ptr removed object
 * @return
 * * ::YADSL_STACK_RET_OK, and *object_ptr is updated
 * * ::YADSL_STACK_RET_EMPTY
*/
yadsl_StackRet
yadsl_stack_item_remove(
	yadsl_StackHandle *stack,
	yadsl_StackItemObj **object_ptr);

/**
 * @brief Destroy stack and its remaining objects
 * @param stack stack
 * @param free_item_func item freeing function
*/
void
yadsl_stack_destroy(
	yadsl_StackHandle *stack,
	yadsl_StackItemFreeFunc free_item_func);

/** }@ */

#endif
