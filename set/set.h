#ifndef __SET_H__
#define __SET_H__

/**
* A Set starts empty.
* You are able to add and remove items (opaque pointers),
* check if items are contained within a set or not,
* and iterate through them.
* It does not acquire the ownership of the items it
* stores and, therefore, does not deallocates them
* when destroyed.
* HINT: SET_RETURN_OK will always be 0, therefore
* it can be used as a boolean value to check if a
* function went OK or not, eg:
* if (setId = setFunction(pSet)) { ... }
* HINT: The set stores items according to their
* address, thus, totally arbitrarily. The list
* does not give any information about the value
* stored at all.
*/

typedef enum
{
	/* All went ok */
	SET_RETURN_OK = 0,

	/* Invalid parameter was provided */
	SET_RETURN_INVALID_PARAMETER,

	/* Could not allocate memory space */
	SET_RETURN_MEMORY,

	/* Set contains item */
	SET_RETURN_CONTAINS,

	/* Set does not contain item */
	SET_RETURN_DOES_NOT_CONTAIN,

	/* Empty set */
	SET_RETURN_EMPTY,

	/* Could not go backwards or forward in list */
	SET_RETURN_OUT_OF_BOUNDS,
	
}
SetReturnID;

typedef struct Set Set;

/**
* Create an empty set
* ppSet	 address of pointer to set
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
* 	- "ppSet" is NULL
* SET_RETURN_MEMORY
*/
SetReturnID setCreate(Set **ppSet);

/**
* Check whether set contains item or not
* pSet	  pointer to set
* item	  item to be consulted
* Possible return values:
* SET_RETURN_INVALID_PARAMETER
* 	- "pSet" is NULL
* SET_RETURN_CONTAINS
* 	- set contains item
* SET_RETURN_DOES_NOT_CONTAIN
* 	- set does not contain item
*/
SetReturnID setContainsItem(Set *pSet, void *item);

/**
* Filter through the set with a filtering function and returns
* the first item that returns with positive response.
* pSet      pointer to set
* func      filtering function: takes an item from the set and
*           an auxiliary argument as parameters, in that order,
*           and returns a boolean.
* arg       auxiliary argument parsed to the filtering function
* pItem     address of pointer that will point to the found item
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
* 	- "pSet" is NULL
* 	- "func" is NULL
* 	- "pItem" is NULL
* SET_RETURN_DOES_NOT_CONTAIN
* 	- set does not contain such item
* [!] The filter function must not alter the set state (like adding,
* deleting or freeing items), since it can corrupt the set
*/
SetReturnID setFilterItem(Set *pSet, int (*func) (void *item, void *arg),
	void *arg, void **pItem);

/**
* Adds item to set
* pSet      pointer to set
* item      item to be added
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
* 	- "pSet" is NULL
* SET_RETURN_CONTAINS
* 	- set already contains item
* SET_RETURN_OVERFLOW
*	- reached the size limit (ULONG_MAX)
* SET_RETURN_MEMORY
*/
SetReturnID setAddItem(Set *pSet, void *item);

/**
* Remove item from set
* pSet  pointer to set
* item	item to be removed
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
* 	- "pSet" is NULL
* SET_RETURN_DOES_NOT_CONTAIN
* 	- set does not contain item
*/
SetReturnID setRemoveItem(Set *pSet, void *item);

/**
* Obtain item currently pointed by the cursor
* pSet      pointer to set
* pItem     address of variable that will hold the item
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
* 	- "pSet" is NULL
* 	- "pItem" is NULL
* SET_RETURN_EMPTY
*/
SetReturnID setGetCurrentItem(Set *pSet, void **pItem);

/**
* Obtain number of items contained in the set
* pSet      pointer to set
* pItem	    address of variable that will hold the value
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
* 	- "pSet" is NULL
* 	- "pValue" is NULL
*/
SetReturnID setGetSize(Set *pSet, unsigned long *pValue);

/**
* Make cursor point to the previous item
* pSet	  pointer to set
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
* 	- "pSet" is NULL
* SET_RETURN_EMPTY
* SET_RETURN_OUT_OF_BOUNDS
* 	- current item is the first in the set
*/
SetReturnID setPreviousItem(Set *pSet);

/**
* Make cursor point to the next item
* pSet     pointer to set
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
* 	- "pSet" is NULL
* SET_RETURN_EMPTY
* SET_RETURN_OUT_OF_BOUNDS
* 	- current item is the last in the set
*/
SetReturnID setNextItem(Set *pSet);

/**
* Make cursor point to the first item
* pSet	  pointer to set
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
* 	- "pSet" is NULL
* SET_RETURN_EMPTY
*/
SetReturnID setFirstItem(Set *pSet);

/**
* Make cursor point to the last item
* pSet  pointer to set
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
* 	- "pSet" is NULL
* SET_RETURN_EMPTY
*/
SetReturnID setLastItem(Set *pSet);

/**
* Free set structure from memory
* pSet  pointer to set
*/
void setDestroy(Set *pSet);

/**
* Free set structure from memory and call a special function
* for each item that is removed, avoiding memory leak
* pSet      pointer to set
* freeItem  function that will be called for every
*           item in the set exactly once
* arg       auxiliary argument
*/
void setDestroyDeep(Set *pSet, void (*freeItem)(void *item, void *arg), void *arg);

#endif
