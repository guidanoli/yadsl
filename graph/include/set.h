#ifndef __SET_H__
#define __SET_H__

/**
* A Set starts empty.
* You are able to add and remove 64-bit values,
* check if values are contained within a set,
* and iterate through them.
*/

typedef enum
{
    /* All went ok */
    SET_RETURN_OK,

    /* Invalid parameter was provided */
    SET_RETURN_INVALID_PARAMETER,

    /* Could not allocate memory space */
    SET_RETURN_MEMORY,

    /* Set contains a number */
    SET_RETURN_CONTAINS,

    /* Set does not contain a number */
    SET_RETURN_DOES_NOT_CONTAIN,

    /* Empty set */
    SET_RETURN_EMPTY,

    /* Could not go backwards of forward in list */
    SET_RETURN_OUT_OF_BOUNDS,
}
SetReturnID;

typedef struct Set Set;

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
* Check whether set contains value or not
* pSet      pointer to set
* value     value to be checked
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
*   - "pSet" is NULL
* SET_RETURN_CONTAINS
*   - set contains value
* SET_RETURN_DOES_NOT_CONTAIN
*   - set does not contain value
*/
SetReturnID setContains(Set *pSet, size_t value);

/**
* Adds value to set
* pSet      pointer to set
* value     value to be added
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
*   - "pSet" is NULL
* SET_RETURN_CONTAINS
*   - set already contains value
* SET_RETURN_MEMORY
*/
SetReturnID setAdd(Set *pSet, size_t value);

/**
* Remove value from set
* pSet      pointer to set
* value     value to be removed
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
*   - "pSet" is NULL
* SET_RETURN_DOES_NOT_CONTAIN
*   - set does not contain value
*/
SetReturnID setRemove(Set *pSet, size_t value);

/**
* Obtain current value pointed by the cursor
* pSet      pointer to set
* pValue    adress of variable that will hold the value
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
*   - "pSet" is NULL
*   - "pValue" is NULL
* SET_RETURN_EMPTY
*/
SetReturnID setGetCurrent(Set *pSet, size_t *pValue);

/**
* Make cursor point to the largest value smaller than
* the one currently pointed to
* pSet      pointer to set
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
*   - "pSet" is NULL
* SET_RETURN_EMPTY
* SET_RETURN_OUT_OF_BOUNDS
*   - current value pointed is the smallest in set
*/
SetReturnID setPreviousValue(Set *pSet);

/**
* Make cursor point to the smallest value larger than
* the one currently pointed to
* pSet      pointer to set
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
*   - "pSet" is NULL
* SET_RETURN_EMPTY
* SET_RETURN_OUT_OF_BOUNDS
*   - current value pointed is the largest in set
*/
SetReturnID setNextValue(Set *pSet);

/**
* Make cursor point to the smallest value in the set
* pSet      pointer to set
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
*   - "pSet" is NULL
* SET_RETURN_EMPTY
*/
SetReturnID setFirstValue(Set *pSet);

/**
* Make cursor point to the largest value in the set
* pSet      pointer to set
* Possible errors:
* SET_RETURN_INVALID_PARAMETER
*   - "pSet" is NULL
* SET_RETURN_EMPTY
*/
SetReturnID setLastValue(Set *pSet);

/**
* Free set structure from memory
* pSet      pointer to set
*/
void setDestroy(Set *pSet);

#endif
