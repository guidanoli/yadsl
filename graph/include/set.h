#ifndef __SET_H__
#define __SET_H__

/**
* A Set starts empty.
* You are able to add and remove 64-bit values
* and check if values are contained within a set.
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
* Free set structure from memory
* pSet      pointer to set
*/
void setDestroy(Set *pSet);

#endif
