#ifndef __YADSL_SET_H__
#define __YADSL_SET_H__

/**
 * \defgroup set Set
 * @brief Generic Set
 * 
 * A Set starts empty.
 * You are able to add and remove items (opaque pointers),
 * check if items are contained within a set or not,
 * and iterate through them.
 * It makes sure no duplicates are added.
 * 
 * It does not acquire the ownership of the items it
 * stores and, therefore, does not deallocates them
 * when destroyed.
 *
 * Items can assume NULL (0) value.
 *
 * The filtering function takes an item and the additional
 * argument as parameters and should return a boolean value
 * indicating if the item is the one to be filtered. If 'True'
 * is returned, the iteration stops and the item is returned
 * by reference (yet, the ownership is still the set's).
 *
 * If the set state is changed while filtering, the function
 * will be recursively. No guarantee is given that the call
 * will terminate.
 * 
 * @{
*/

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Value returned by the Set functions
*/
typedef enum
{
	YADSL_SET_RET_OK = 0, /**< All went ok */
	YADSL_SET_RET_MEMORY, /**< Could not allocate memory */
	YADSL_SET_RET_CONTAINS, /**< Set contains item */
	YADSL_SET_RET_DOES_NOT_CONTAIN, /**< Set does not contain item */
	YADSL_SET_RET_EMPTY, /**< Set is empty */
	YADSL_SET_RET_OUT_OF_BOUNDS, /**< Iteration cursor got out of bounds */
}
yadsl_SetRet;

typedef void yadsl_SetHandle; /**< Set handle */
typedef void yadsl_SetItemObj; /**< Set item object */

typedef void yadsl_SetItemFilterArg; /**< Argument passed to yadsl_SetFilterFunc */
typedef void yadsl_SetItemFreeArg; /**< Argument passed to yadsl_SetItemFreeFunc */

/**
 * @brief Function responsible for filtering items in a set
 * @param item set item
 * @param arg user argument
 * @return whether item matches (and filtering must stop) or not
*/
typedef bool
(*yadsl_SetItemFilterFunc) (
	yadsl_SetItemObj* item,
	yadsl_SetItemFilterArg* arg);

/**
 * @brief Function responsible for freeing items in a set
 * @param item set item
 * @param arg user argument
*/
typedef void
(*yadsl_SetItemFreeFunc)(
	yadsl_SetItemObj* item,
	yadsl_SetItemFreeArg* arg);

//  =========== ============================ 
//   setCreate   Create an empty set         
//  =========== ============================ 
//   ppSet       (owned ret) pointer to set  
//  =========== ============================ 
//  [!] YADSL_SET_RET_MEMORY

yadsl_SetHandle*
yadsl_set_create();

//  ================= ======================================== 
//   setContainsItem   Check whether set contains item or not  
//  ================= ======================================== 
//   set              pointer to set                          
//   item              item to be checked                      
//  ================= ======================================== 
//  [!] YADSL_SET_RET_CONTAINS
//  [!] YADSL_SET_RET_DOES_NOT_CONTAIN

yadsl_SetRet
yadsl_set_item_contains_check(
	yadsl_SetHandle *set,
	void *item);

//  =============== ==================================== 
//   setFilterItem      Filter item from set             
//  =============== ==================================== 
//   set            pointer to set                     
//   func            filtering function                
//   arg             (opt) additional argument to func   
//   pItem           (ret) filtered item                 
//  =============== ==================================== 
//  [!] YADSL_SET_RET_DOES_NOT_CONTAIN: no item filtered

yadsl_SetRet yadsl_set_item_filter(
	yadsl_SetHandle *set,
	yadsl_SetItemFilterFunc item_filter_func,
	yadsl_SetItemFilterArg * item_filter_arg,
	yadsl_SetItemObj **item_ptr);

//  ============ ========================== 
//   setAddItem   Add item to set           
//  ============ ========================== 
//   set         pointer to set           
//   item         (owned) item to be added  
//  ============ ========================== 
//  [!] YADSL_SET_RET_CONTAINS
//  [!] YADSL_SET_RET_MEMORY

yadsl_SetRet
yadsl_set_item_add(
	yadsl_SetHandle *set,
	yadsl_SetItemObj *item);

//  =============== ====================== 
//   setRemoveItem   Remove item from set  
//  =============== ====================== 
//   set            pointer to set        
//   item            item to be removed    
//  =============== ====================== 
//  [!] YADSL_SET_RET_DOES_NOT_CONTAIN

yadsl_SetRet
yadsl_set_item_remove(
	yadsl_SetHandle *set,
	yadsl_SetItemObj *item);

//  =================== ========================================== 
//   setGetCurrentItem   Get item currently pointed by the cursor  
//  =================== ========================================== 
//   set                pointer to set                            
//   pItem               (ret) item pointed by the cursor          
//  =================== ========================================== 
//  [!] YADSL_SET_RET_EMPTY

yadsl_SetRet
yadsl_set_cursor_get(
	yadsl_SetHandle *set,
	yadsl_SetItemObj **item_ptr);

//  ============ ======================= 
//   setGetSize    Get set cardinality   
//  ============ ======================= 
//   set         pointer to set         
//   pSize        (ret) set cardinality  
//  ============ ======================= 

yadsl_SetRet
yadsl_set_size_get(
	yadsl_SetHandle *set,
	size_t *size_ptr);

//  ================= ======================================== 
//   setPreviousItem   Make cursor point to the previous item  
//  ================= ======================================== 
//   set              pointer to set                          
//  ================= ======================================== 
//  [!] YADSL_SET_RET_EMPTY
//  [!] YADSL_SET_RET_OUT_OF_BOUNDS: current item is the first in the set

yadsl_SetRet
yadsl_set_cursor_previous(
	yadsl_SetHandle *set);

//  ============= ==================================== 
//   setNextItem   Make cursor point to the next item  
//  ============= ==================================== 
//   set          pointer to set                      
//  ============= ==================================== 
//  [!] YADSL_SET_RET_EMPTY
//  [!] YADSL_SET_RET_OUT_OF_BOUNDS: current item is the last in the set

yadsl_SetRet
yadsl_set_cursor_next(
	yadsl_SetHandle *set);

//  ============== ===================================== 
//   setFirstItem   Make cursor point to the first item  
//  ============== ===================================== 
//   set           pointer to set                       
//  ============== ===================================== 
//  [!] YADSL_SET_RET_EMPTY

yadsl_SetRet
yadsl_set_cursor_first(
	yadsl_SetHandle *set);

//  ============= ==================================== 
//   setLastItem   Make cursor point to the last item  
//  ============= ==================================== 
//   set          pointer to set                      
//  ============= ==================================== 
//  [!] YADSL_SET_RET_EMPTY

yadsl_SetRet
yadsl_set_cursor_last(
	yadsl_SetHandle *set);

//  ============ ======================================== 
//   setDestroy   Deallocate set and its remaining items  
//  ============ ======================================== 
//   set         pointer to set                          
//   freeItem     item deallocation function              
//   arg          additional argument passed to freeItem  
//  ============ ======================================== 

void
yadsl_set_destroy(
	yadsl_SetHandle *set,
	yadsl_SetItemFreeFunc free_item_func,
	yadsl_SetItemFreeArg *free_item_arg);

#endif
