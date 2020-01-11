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
*/

typedef enum
{
    /* All went ok */
    SET_RETURN_OK,

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

/*
* Macros to ensure compatiblity with previous versions
* of the set module -- when only numbers where stored
*/

#define setContains(p, v)       setContainsItem(p, (void *) v)
#define setAdd(p, v)            setAddItem(p, (void *) v)
#define setRemove(p, v)         setRemoveItem(p, (void *) v)
#define setGetCurrent(p, pV)    setGetCurrentItem(p, (void **) pV)
#define setPreviousValue(p)     setPreviousItem(p)
#define setNextValue(p)         setNextItem(p)
#define setFirstValue(p)        setFirstItem(p)
#define setLastValue(p)         setLastItem(p)

/**
* Create an empty set
* ppSet     address of pointer to set
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
*   - "ppSet" is NULL
* SET_RETURN_MEMORY
*/
SetReturnID setCreate(Set **ppSet);

/**
* Check whether set contains item or not
* pSet      pointer to set
* item      item to be consulted
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
*   - "pSet" is NULL
* SET_RETURN_CONTAINS
*   - set contains item
* SET_RETURN_DOES_NOT_CONTAIN
*   - set does not contain item
*/
SetReturnID setContainsItem(Set *pSet, void *item);

/**
* Adds item to set
* pSet      pointer to set
* item      item to be added
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
*   - "pSet" is NULL
* SET_RETURN_CONTAINS
*   - set already contains item
* SET_RETURN_MEMORY
* [!] This will make the cursor point to the newly added item
*/
SetReturnID setAddItem(Set *pSet, void *item);

/**
* Remove item from set
* pSet      pointer to set
* item      item to be removed
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
*   - "pSet" is NULL
* SET_RETURN_DOES_NOT_CONTAIN
*   - set does not contain item
* [!] This can alter the cursor
*/
SetReturnID setRemoveItem(Set *pSet, void *item);

/**
* Obtain current item pointed by the cursor
* pSet      pointer to set
* pItem     adress of variable that will hold the item
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
*   - "pSet" is NULL
*   - "pItem" is NULL
* SET_RETURN_EMPTY
*/
SetReturnID setGetCurrentItem(Set *pSet, void **pItem);

/**
* Obtain number of items contained in the set
* pSet      pointer to set
* pItem     adress of variable that will hold the value
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
*   - "pSet" is NULL
*   - "pValue" is NULL
*/
SetReturnID setGetSize(Set *pSet, unsigned long *pValue);

/**
* Make cursor point to the previous item
* pSet      pointer to set
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
*   - "pSet" is NULL
* SET_RETURN_EMPTY
* SET_RETURN_OUT_OF_BOUNDS
*   - current item is the first in the set
*/
SetReturnID setPreviousItem(Set *pSet);

/**
* Make cursor point to the next item
* pSet      pointer to set
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
*   - "pSet" is NULL
* SET_RETURN_EMPTY
* SET_RETURN_OUT_OF_BOUNDS
*   - current item is the last in the set
*/
SetReturnID setNextItem(Set *pSet);

/**
* Make cursor point to the first item
* pSet      pointer to set
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
*   - "pSet" is NULL
* SET_RETURN_EMPTY
*/
SetReturnID setFirstItem(Set *pSet);

/**
* Make cursor point to the last item
* pSet      pointer to set
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
*   - "pSet" is NULL
* SET_RETURN_EMPTY
*/
SetReturnID setLastItem(Set *pSet);

/**
* Free set structure from memory
* pSet      pointer to set
*/
void setDestroy(Set *pSet);

#endif
